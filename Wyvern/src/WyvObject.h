#ifndef _H_WYVOBJECT_
#define _H_WYVOBJECT_

#include <memory>

namespace wyv
{
	class WyvObject;
	typedef std::shared_ptr<WyvObject> SharedObject;
	class WyvObject
	{
	public:
		WyvObject() {}
		~WyvObject() {}

		static SharedObject CreateShared() { return std::make_shared<WyvObject>(); }
	};
}

#endif //_H_WYVOBJECT_