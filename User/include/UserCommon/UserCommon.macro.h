#pragma once

#if defined(_DEBUG) || defined(DEBUG)
#define ASSERT(p) assert(p)
#else 
#define ASSERT( p ) (void)0;
#endif // DEBUG

#define MAKE_GETTER(Value, Name) const decltype(Value)& Name = Value