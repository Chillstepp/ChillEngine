#pragma comment(lib, "engine.lib")

#define TEST_ENTITY_COMPONENTS 0
#define TEST_RENDERER 1

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_RENDERER
#include "TestRenderer.h"
#else
#error One of the tests need to be enabled
#endif

#include <crtdbg.h>

#ifdef _WIN64
#include <Windows.h>

int main()
{
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    engine_test test{};
    if(test.initialize())
    {
        test.run();
    }
    
    test.shutdown();
    
    return 0;
}
#endif // _WIN64