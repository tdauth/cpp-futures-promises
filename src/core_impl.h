#ifndef ADV_CORE_IMPL_H
#define ADV_CORE_IMPL_H

#include "core.h"
#include "mvar/core.h"

namespace adv
{

template <typename T>
template <typename S>
typename Core<S>::SharedPtr Core<T>::createShared(Executor *executor,
                                                  Implementation implementation)
{
	// TODO Support different implementations like CAS and STM
	switch (implementation)
	{
		case MVar:
			return typename Core<S>::SharedPtr(new adv_mvar::Core<S>(executor));
	}

	throw std::runtime_error("Invalid implementation");
}

} // namespace adv

#endif