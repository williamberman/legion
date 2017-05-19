/* Copyright 2017 Stanford University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "inst_impl.h"

#include "event_impl.h"
#include "mem_impl.h"
#include "logging.h"
#include "runtime_impl.h"

namespace Realm {

  Logger log_inst("inst");

#ifdef OLD_ALLOCATORS
  ////////////////////////////////////////////////////////////////////////
  //
  // class DeferredInstDestroy
  //

    class DeferredInstDestroy : public EventWaiter {
    public:
      DeferredInstDestroy(RegionInstanceImpl *i) : impl(i) { }
      virtual ~DeferredInstDestroy(void) { }
    public:
      virtual bool event_triggered(Event e, bool poisoned)
      {
	// if input event is poisoned, do not attempt to destroy the lock
	// we don't have an output event here, so this may result in a leak if nobody is
	//  paying attention
	if(poisoned) {
	  log_poison.info() << "poisoned deferred instance destruction skipped - POSSIBLE LEAK - inst=" << impl->me;
	} else {
	  log_inst.info("instance destroyed: space=" IDFMT " id=" IDFMT "",
			impl->metadata.is.id, impl->me.id);
	  get_runtime()->get_memory_impl(impl->memory)->destroy_instance(impl->me, true); 
	}
        return true;
      }

      virtual void print(std::ostream& os) const
      {
        os << "deferred instance destruction";
      }

      virtual Event get_finish_event(void) const
      {
	return Event::NO_EVENT;
      }

    protected:
      RegionInstanceImpl *impl;
    };
#endif
  
  ////////////////////////////////////////////////////////////////////////
  //
  // class RegionInstance
  //

    AddressSpace RegionInstance::address_space(void) const
    {
      return ID(id).instance.owner_node;
    }

    Memory RegionInstance::get_location(void) const
    {
      RegionInstanceImpl *i_impl = get_runtime()->get_instance_impl(*this);
      return i_impl->memory;
    }

    /*static*/ Event RegionInstance::create_instance(RegionInstance& inst,
						     Memory memory,
						     InstanceLayoutGeneric *ilg,
						     const ProfilingRequestSet& prs,
						     Event wait_on)
    {
      MemoryImpl *m_impl = get_runtime()->get_memory_impl(memory);
      RegionInstanceImpl *impl = m_impl->new_instance();

      impl->metadata.layout = ilg;
      
      if (!prs.empty()) {
        impl->requests = prs;
        impl->measurements.import_requests(impl->requests);
        if(impl->measurements.wants_measurement<ProfilingMeasurements::InstanceTimeline>())
          impl->timeline.record_create_time();
      }

      // request allocation of storage - a true response means it was serviced right
      //  away
      Event ready_event;
      if(m_impl->allocate_instance_storage(impl->me,
					   ilg->bytes_used,
					   ilg->alignment_reqd,
					   wait_on)) {
	assert(impl->metadata.inst_offset != (size_t)-1);
	ready_event = Event::NO_EVENT;
        if(impl->measurements.wants_measurement<ProfilingMeasurements::InstanceTimeline>())
          impl->timeline.record_ready_time();
      } else {
	// we will probably need an event to track when it is ready
	GenEventImpl *ev = GenEventImpl::create_genevent();
	ready_event = ev->current_event();
	bool alloc_done;
	// use mutex to avoid race on allocation callback
	{
	  AutoHSLLock al(impl->mutex);
	  if(impl->metadata.inst_offset != (size_t)-1) {
	    alloc_done = true;
	  } else {
	    alloc_done = false;
	    impl->metadata.ready_event = ready_event;
	  }
	}
	if(alloc_done) {
	  if(impl->measurements.wants_measurement<ProfilingMeasurements::InstanceTimeline>())
	    impl->timeline.record_ready_time();
	  GenEventImpl::trigger(ready_event, false /*!poisoned*/);
	  ready_event = Event::NO_EVENT;
	}
      }

      inst = impl->me;
      return ready_event;
    }

    void RegionInstance::destroy(Event wait_on /*= Event::NO_EVENT*/) const
    {
      // we can immediately turn this into a (possibly-preconditioned) request to
      //  deallocate the instance's storage - the eventual callback from that
      //  will be what actually destroys the instance
      DetailedTimer::ScopedPush sp(TIME_LOW_LEVEL);
      // this does the right thing even though we're using an instance ID
      MemoryImpl *mem_impl = get_runtime()->get_memory_impl(*this);
      mem_impl->release_instance_storage(*this, wait_on);
#ifdef OLD_ALLOCATORS
      RegionInstanceImpl *i_impl = get_runtime()->get_instance_impl(*this);
      if (!wait_on.has_triggered())
      {
	EventImpl::add_waiter(wait_on, new DeferredInstDestroy(i_impl));
        return;
      }

      log_inst.info("instance destroyed: space=" IDFMT " id=" IDFMT "",
	       i_impl->metadata.is.id, this->id);
      get_runtime()->get_memory_impl(i_impl->memory)->destroy_instance(*this, true);
#endif
    }

    void RegionInstance::destroy(const std::vector<DestroyedField>& destroyed_fields,
				 Event wait_on /*= Event::NO_EVENT*/) const
    {
      // TODO: actually call destructor
      assert(destroyed_fields.empty());
      destroy(wait_on);
    }

    /*static*/ const RegionInstance RegionInstance::NO_INST = { 0 };

    // a generic accessor just holds a pointer to the impl and passes all 
    //  requests through
    LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic> RegionInstance::get_accessor(void) const
    {
      // request metadata (if needed), but don't block on it yet
      RegionInstanceImpl *i_impl = get_runtime()->get_instance_impl(*this);
      Event e = i_impl->metadata.request_data(ID(id).instance.owner_node, id);
      if(!e.has_triggered())
	log_inst.info("requested metadata in accessor creation: " IDFMT, id);
	
      return LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic>(LegionRuntime::Accessor::AccessorType::Generic::Untyped((void *)i_impl));
    }

#if 0
    /*static*/ RegionInstance RegionInstance::create_instance(Memory memory,
							      const LinearizedIndexSpaceIntfc& lis,
							      const std::vector<size_t>& field_sizes,
							      const ProfilingRequestSet& prs)
    {
      size_t num_elements = lis.size();
      size_t element_size = 0;
      for(std::vector<size_t>::const_iterator it = field_sizes.begin();
	  it != field_sizes.end();
	  it++)
	element_size += *it;

      MemoryImpl *m_impl = get_runtime()->get_memory_impl(memory);

      int dummy_bits[RegionInstanceImpl::MAX_LINEARIZATION_LEN];
      for(size_t i = 0; i < RegionInstanceImpl::MAX_LINEARIZATION_LEN; i++)
	dummy_bits[i] = 0;

      RegionInstance r = m_impl->create_instance(IndexSpace::NO_SPACE,
						 dummy_bits,
						 num_elements * element_size,
						 num_elements, // SOA
						 element_size,
						 field_sizes,
						 0, -1, prs,
						 RegionInstance::NO_INST);
			
      RegionInstanceImpl *r_impl = get_runtime()->get_instance_impl(r);
      r_impl->lis = lis.clone();
			 
      return r;
    }
#endif

    const LinearizedIndexSpaceIntfc& RegionInstance::get_lis(void) const
    {
      RegionInstanceImpl *r_impl = get_runtime()->get_instance_impl(*this);
      assert(r_impl->lis);
      return *(r_impl->lis);
    }

    const InstanceLayoutGeneric *RegionInstance::get_layout(void) const
    {
      RegionInstanceImpl *r_impl = get_runtime()->get_instance_impl(*this);
      // TODO: wait for metadata to be valid?
      assert(r_impl->metadata.layout);
      return r_impl->metadata.layout;
    }

    void *RegionInstance::get_base_address(void) const
    {
      RegionInstanceImpl *r_impl = get_runtime()->get_instance_impl(*this);
      // TODO: wait for metadata to be valid?
      assert(r_impl->metadata.layout);
      MemoryImpl *mem = get_runtime()->get_memory_impl(r_impl->memory);
      return mem->get_direct_ptr(r_impl->metadata.inst_offset,
				 r_impl->metadata.layout->bytes_used);
    }

    void RegionInstance::get_strided_access_parameters(size_t start, size_t count,
						       ptrdiff_t field_offset, size_t field_size,
						       intptr_t& base, ptrdiff_t& stride)
    {
      RegionInstanceImpl *r_impl = get_runtime()->get_instance_impl(*this);

      // TODO: make sure we're in range

      void *orig_base = 0;
      size_t orig_stride = 0;
      bool ok = r_impl->get_strided_parameters(orig_base, orig_stride, field_offset);
      assert(ok);
      base = reinterpret_cast<intptr_t>(orig_base);
      stride = orig_stride;
    }

    void RegionInstance::report_instance_fault(int reason,
					       const void *reason_data,
					       size_t reason_size) const
    {
      assert(0);
    }

  
  ////////////////////////////////////////////////////////////////////////
  //
  // class RegionInstanceImpl
  //

#ifdef OLD_ALLOCATORS
    RegionInstanceImpl::RegionInstanceImpl(RegionInstance _me, IndexSpace _is, Memory _memory, 
					   off_t _offset, size_t _size, ReductionOpID _redopid,
					   const DomainLinearization& _linear, size_t _block_size,
					   size_t _elmt_size, const std::vector<size_t>& _field_sizes,
					   const ProfilingRequestSet &reqs,
					   off_t _count_offset /*= 0*/, off_t _red_list_size /*= 0*/,
					   RegionInstance _parent_inst /*= NO_INST*/)
      : me(_me), memory(_memory), lis(0)
    {
      metadata.linearization = _linear;

      metadata.block_size = _block_size;
      metadata.elmt_size = _elmt_size;

      metadata.field_sizes = _field_sizes;

      metadata.is = _is;
      metadata.alloc_offset = _offset;
      //metadata.access_offset = _offset + _adjust;
      metadata.size = _size;
      
      //StaticAccess<IndexSpaceImpl> rdata(_is.impl());
      //locked_data.first_elmt = rdata->first_elmt;
      //locked_data.last_elmt = rdata->last_elmt;

      metadata.redopid = _redopid;
      metadata.count_offset = _count_offset;
      metadata.red_list_size = _red_list_size;
      metadata.parent_inst = _parent_inst;

      metadata.mark_valid();

      lock.init(ID(me).convert<Reservation>(), ID(me).instance.owner_node);
      lock.in_use = true;

      if (!reqs.empty()) {
        requests = reqs;
        measurements.import_requests(requests);
        if (measurements.wants_measurement<
                          ProfilingMeasurements::InstanceTimeline>()) {
          timeline.record_create_time();
        }
      }
    }
#endif

    RegionInstanceImpl::RegionInstanceImpl(RegionInstance _me, Memory _memory)
      : me(_me), memory(_memory), lis(0)
    {
      lock.init(ID(me).convert<Reservation>(), ID(me).instance.creator_node);
      lock.in_use = true;

      metadata.inst_offset = (size_t)-1;
      metadata.ready_event = Event::NO_EVENT;
      metadata.layout = 0;
    }

    RegionInstanceImpl::~RegionInstanceImpl(void) {}

    void RegionInstanceImpl::notify_allocation(bool success, size_t offset)
    {
      log_inst.debug() << "allocation completed: inst=" << me << " offset=" << offset;
      assert(success);

      // before we publish the offset, we need to update the layout
      // SJT: or not?  that might be part of RegionInstance::get_base_address?
      //metadata.layout->relocate(offset);

      // update must be performed with the metadata mutex held to make sure there
      //  are no races between it and getting the ready event 
      Event ready_event;
      {
	AutoHSLLock al(mutex);
	ready_event = metadata.ready_event;
	metadata.ready_event = Event::NO_EVENT;
	metadata.inst_offset = offset;
      }
      if(ready_event.exists())
	GenEventImpl::trigger(ready_event, !success);

      // metadata is now valid and can be shared
      NodeSet early_reqs;
      metadata.mark_valid(early_reqs);
      if(!early_reqs.empty()) {
	log_inst.debug() << "sending instance metadata to early requestors: isnt=" << me;
	size_t datalen = 0;
	void *data = metadata.serialize(datalen);
	MetadataResponseMessage::broadcast_request(early_reqs, ID(me).id, data, datalen);
	free(data);
      }
    }

    void RegionInstanceImpl::notify_deallocation(void)
    {
      log_inst.debug() << "deallocation completed: inst=" << me;
    }

    // helper function to figure out which field we're in
    void find_field_start(const std::vector<size_t>& field_sizes, off_t byte_offset,
			  size_t size, off_t& field_start, int& field_size)
    {
      off_t start = 0;
      for(std::vector<size_t>::const_iterator it = field_sizes.begin();
	  it != field_sizes.end();
	  it++) {
	assert((*it) > 0);
	if(byte_offset < (off_t)(*it)) {
	  if ((off_t)(byte_offset + size) > (off_t)(*it)) {
            log_inst.error("Requested field does not match the expected field size");
            assert(false);
          }
	  field_start = start;
	  field_size = (*it);
	  return;
	}
	start += (*it);
	byte_offset -= (*it);
      }
      assert(0);
    }

    void RegionInstanceImpl::record_instance_usage(void)
    {
      // can't do this in the constructor because our ID isn't right yet...
      if(measurements.wants_measurement<ProfilingMeasurements::InstanceMemoryUsage>()) {
	ProfilingMeasurements::InstanceMemoryUsage usage;
	usage.instance = me;
	usage.memory = memory;
	usage.bytes = metadata.size;
	measurements.add_measurement(usage);
      }
    }

    bool RegionInstanceImpl::get_strided_parameters(void *&base, size_t &stride,
						      off_t field_offset)
    {
      MemoryImpl *mem = get_runtime()->get_memory_impl(memory);

      // must have valid data by now - block if we have to
      metadata.await_data();

      off_t offset = metadata.alloc_offset;
      size_t elmt_stride;
      
      if (metadata.block_size == 1) {
        offset += field_offset;
        elmt_stride = metadata.elmt_size;
      } else {
        off_t field_start=0;
        int field_size=0;
        find_field_start(metadata.field_sizes, field_offset, 1, field_start, field_size);

        offset += (field_start * metadata.block_size) + (field_offset - field_start);
	elmt_stride = field_size;
      }

      base = mem->get_direct_ptr(offset, 0);
      if (!base) return false;

      // if the caller wants a particular stride and we differ (and have more
      //  than one element), fail
      if(stride != 0) {
        if((stride != elmt_stride) && (metadata.size > metadata.elmt_size))
          return false;
      } else {
        stride = elmt_stride;
      }

      // if there's a per-element offset, apply it after we've agreed with the caller on 
      //  what we're pretending the stride is
      const DomainLinearization& dl = metadata.linearization;
      if(dl.get_dim() > 0) {
	// make sure this instance uses a 1-D linearization
	assert(dl.get_dim() == 1);

	LegionRuntime::Arrays::Mapping<1, 1> *mapping = dl.get_mapping<1>();
	LegionRuntime::Arrays::Rect<1> preimage = mapping->preimage((coord_t)0);
	assert(preimage.lo == preimage.hi);
	// double-check that whole range maps densely
	preimage.hi.x[0] += 1; // not perfect, but at least detects non-unit-stride case
	assert(mapping->image_is_dense(preimage));
	coord_t inst_first_elmt = preimage.lo[0];
	//printf("adjusting base by %d * %zd: %p -> %p\n", inst_first_elmt, stride,
	//       base,
	//       ((char *)base) - inst_first_elmt * stride);
	base = ((char *)base) - inst_first_elmt * stride;
      }

      return true;
    }

    void RegionInstanceImpl::finalize_instance(void)
    {
      if (!requests.empty()) {
        if (measurements.wants_measurement<
                          ProfilingMeasurements::InstanceTimeline>()) {
	  // set the instance ID correctly now - it wasn't available at construction time
          timeline.instance = me;
          timeline.record_delete_time();
          measurements.add_measurement(timeline);
        }
        measurements.send_responses(requests);
        requests.clear();
      }
    }

    void *RegionInstanceImpl::Metadata::serialize(size_t& out_size) const
    {
      // figure out how much space we need
      out_size = (sizeof(IndexSpace) +
		  sizeof(off_t) +
		  sizeof(size_t) +
		  sizeof(ReductionOpID) +
		  sizeof(off_t) +
		  sizeof(off_t) +
		  sizeof(size_t) +
		  sizeof(size_t) +
		  sizeof(size_t) + (field_sizes.size() * sizeof(size_t)) +
		  sizeof(RegionInstance) +
		  (MAX_LINEARIZATION_LEN * sizeof(int)));
      void *data = malloc(out_size);
      char *pos = (char *)data;
#define S(val) do { memcpy(pos, &(val), sizeof(val)); pos += sizeof(val); } while(0)
      S(is);
      S(alloc_offset);
      S(size);
      S(redopid);
      S(count_offset);
      S(red_list_size);
      S(block_size);
      S(elmt_size);
      size_t l = field_sizes.size();
      S(l);
      for(size_t i = 0; i < l; i++) S(field_sizes[i]);
      S(parent_inst);
      linearization.serialize((int *)pos);
#undef S
      return data;
    }

    void RegionInstanceImpl::Metadata::deserialize(const void *in_data, size_t in_size)
    {
      const char *pos = (const char *)in_data;
#define S(val) do { memcpy(&(val), pos, sizeof(val)); pos += sizeof(val); } while(0)
      S(is);
      S(alloc_offset);
      S(size);
      S(redopid);
      S(count_offset);
      S(red_list_size);
      S(block_size);
      S(elmt_size);
      size_t l;
      S(l);
      field_sizes.resize(l);
      for(size_t i = 0; i < l; i++) S(field_sizes[i]);
      S(parent_inst);
      linearization.deserialize((const int *)pos);
#undef S
    }

#ifdef POINTER_CHECKS
    void RegionInstanceImpl::verify_access(unsigned ptr)
    {
      StaticAccess<RegionInstanceImpl> data(this);
      const ElementMask &mask = data->is.get_valid_mask();
      if (!mask.is_set(ptr))
      {
        fprintf(stderr,"ERROR: Accessing invalid pointer %d in logical region " IDFMT "\n",ptr,data->is.id);
        assert(false);
      }
    }
#endif

    static inline off_t calc_mem_loc(off_t alloc_offset, off_t field_start, int field_size, int elmt_size,
				     int block_size, int index)
    {
      return (alloc_offset +                                      // start address
	      ((index / block_size) * block_size * elmt_size) +   // full blocks
	      (field_start * block_size) +                        // skip other fields
	      ((index % block_size) * field_size));               // some some of our fields within our block
    }

    void RegionInstanceImpl::get_bytes(int index, off_t byte_offset, void *dst, size_t size)
    {
      // must have valid data by now - block if we have to
      metadata.await_data();
      off_t o;
      if(metadata.block_size == 1) {
	// no blocking - don't need to know about field boundaries
	o = metadata.alloc_offset + (index * metadata.elmt_size) + byte_offset;
      } else {
	off_t field_start=0;
	int field_size=0;
	find_field_start(metadata.field_sizes, byte_offset, size, field_start, field_size);
        o = calc_mem_loc(metadata.alloc_offset, field_start, field_size,
                         metadata.elmt_size, metadata.block_size, index);

      }
      MemoryImpl *m = get_runtime()->get_memory_impl(memory);
      m->get_bytes(o, dst, size);
    }

    void RegionInstanceImpl::put_bytes(int index, off_t byte_offset, const void *src, size_t size)
    {
      // must have valid data by now - block if we have to
      metadata.await_data();
      off_t o;
      if(metadata.block_size == 1) {
	// no blocking - don't need to know about field boundaries
	o = metadata.alloc_offset + (index * metadata.elmt_size) + byte_offset;
      } else {
	off_t field_start=0;
	int field_size=0;
	find_field_start(metadata.field_sizes, byte_offset, size, field_start, field_size);
        o = calc_mem_loc(metadata.alloc_offset, field_start, field_size,
                         metadata.elmt_size, metadata.block_size, index);
      }
      MemoryImpl *m = get_runtime()->get_memory_impl(memory);
      m->put_bytes(o, src, size);
    }

}; // namespace Realm
