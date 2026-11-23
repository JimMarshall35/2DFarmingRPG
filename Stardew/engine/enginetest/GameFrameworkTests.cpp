#include <gtest/gtest.h>
#include "GameFramework.h"
#include <string.h>

struct TestVars
{
    struct CallAndOrder
    {
        CallAndOrder(TestVars* pVars)
            : pParent(pVars)
        {

        }
        TestVars* pParent;
        bool bCalled = false;
        int nOrderCalled = 0;
        void RegisterCall()
        {
            bCalled = true;
            nOrderCalled = (*pParent->pOnCall)++;
        }
    };
    TestVars(int* onCall)
        :update(this),
        draw(this),
        input(this),
        onpush(this),
        onpop(this),
        onWindowDimsChanged(this),
        windowW(0),
        windowH(0),
        pOnCall(onCall)
    {
    }
    CallAndOrder update;
    CallAndOrder draw;
    CallAndOrder input;
    CallAndOrder onpush;
    CallAndOrder onpop;
    CallAndOrder onWindowDimsChanged;
    int windowW;
    int windowH;
    int* pOnCall; // global call counter
};



void TestUpdate(struct GameFrameworkLayer* pLayer, float deltaT)
{
    TestVars* pVars = (TestVars*)pLayer->userData;
    pVars->update.RegisterCall();
}

void TestInput(struct GameFrameworkLayer* pLayer, InputContext* context)
{
    TestVars* pVars = (TestVars*)pLayer->userData;
    pVars->input.RegisterCall();
}

void TestDraw(struct GameFrameworkLayer* pLayer, DrawContext* context)
{
    TestVars* pVars = (TestVars*)pLayer->userData;
    pVars->draw.RegisterCall();
}

void TestOnPush(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    TestVars* pVars = (TestVars*)pLayer->userData;
    pVars->onpush.RegisterCall();
}

void TestOnPop(struct GameFrameworkLayer* pLayer, DrawContext* drawContext, InputContext* inputContext)
{
    TestVars* pVars = (TestVars*)pLayer->userData;
    pVars->onpop.RegisterCall();
}

void TestOnWindowDimsChanged(struct GameFrameworkLayer* pLayer, int newW, int newH)
{
    TestVars* pVars = (TestVars*)pLayer->userData;
    pVars->onWindowDimsChanged.RegisterCall();
    pVars->windowH = newH;
    pVars->windowW = newW;
}

struct GameFrameworkLayer GetTestLayer(int& callCounter)
{
    struct GameFrameworkLayer l;
    memset(&l, 0, sizeof(struct GameFrameworkLayer));
    l.draw = &TestDraw;
    l.update = &TestUpdate;
    l.input = &TestInput;
    l.onPop = &TestOnPop;
    l.onPush = &TestOnPush;
    l.onWindowDimsChanged = &TestOnWindowDimsChanged;
    l.userData = new TestVars(&callCounter);
    return l;
}

void FreeTestLayer(struct GameFrameworkLayer& l)
{
    delete (TestVars*)l.userData;
}

struct ScopedGameFramework
{
    ScopedGameFramework()
    {
        GF_InitGameFramework();
    }
    ~ScopedGameFramework()
    {
        GF_DestroyGameFramework();
    }
};

struct ScopedTestLayer
{
    ScopedTestLayer(int& callCounter)
        :layer(GetTestLayer(callCounter))
    {

    }
    ~ScopedTestLayer()
    {
        FreeTestLayer(layer);
    }
    inline TestVars& Vars()
    {
        return *((TestVars*)layer.userData);
    }
    struct GameFrameworkLayer layer;
};


TEST(GameFramework, NoFunctionsEnabledNoFunctionsCalled)
{
    int callCounter = 0;
    ScopedTestLayer l{callCounter};

    {
        ScopedGameFramework gf;
        GF_PushGameFrameworkLayer(&l.layer);
        GF_EndFrame(nullptr, nullptr);

        GF_UpdateGameFramework(0.0f);
        GF_InputGameFramework(nullptr);
        GF_DrawGameFramework(nullptr);
        GF_PopGameFrameworkLayer();
        GF_EndFrame(nullptr, nullptr);
    }
    

    ASSERT_EQ(false, l.Vars().update.bCalled);
    ASSERT_EQ(false, l.Vars().draw.bCalled);
    ASSERT_EQ(false, l.Vars().input.bCalled);
    ASSERT_EQ(false, l.Vars().onpush.bCalled);
    ASSERT_EQ(false, l.Vars().onpop.bCalled);

    ASSERT_EQ(0, callCounter);
}

TEST(GameFramework, FunctionsEnabledFunctionsCalled)
{
    int callCounter = 0;
    ScopedTestLayer l{callCounter};

    l.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPush | EnableOnPop);

    {
        ScopedGameFramework gf;
        GF_PushGameFrameworkLayer(&l.layer);
        GF_EndFrame(nullptr, nullptr);

        GF_UpdateGameFramework(0.0f);
        GF_InputGameFramework(nullptr);
        GF_DrawGameFramework(nullptr);
        GF_PopGameFrameworkLayer();
        GF_EndFrame(nullptr, nullptr);
    }
    
    ASSERT_EQ(true, l.Vars().update.bCalled);
    ASSERT_EQ(true, l.Vars().draw.bCalled);
    ASSERT_EQ(true, l.Vars().input.bCalled);
    ASSERT_EQ(true, l.Vars().onpush.bCalled);
    ASSERT_EQ(true, l.Vars().onpop.bCalled);

    ASSERT_EQ(5, callCounter);

}

TEST(GameFramework, FunctionsCalledInCorrectOrder)
{
    int callCounter = 0;
    ScopedTestLayer l1{callCounter};
    ScopedTestLayer l2{callCounter};
    ScopedTestLayer l3{callCounter};

    l1.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);
    l2.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);
    l3.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);

    {
        ScopedGameFramework gf;
        GF_PushGameFrameworkLayer(&l1.layer);
        GF_PushGameFrameworkLayer(&l2.layer);
        GF_PushGameFrameworkLayer(&l3.layer);
        GF_EndFrame(nullptr, nullptr);
        GF_UpdateGameFramework(0.0f);
        ASSERT_EQ(3, callCounter);
        callCounter = 0;
        GF_InputGameFramework(nullptr);
        ASSERT_EQ(3, callCounter);
        callCounter = 0;
        GF_DrawGameFramework(nullptr);
        ASSERT_EQ(3, callCounter);
    }

    
    ASSERT_EQ(true, l1.Vars().update.bCalled);
    ASSERT_EQ(true, l2.Vars().update.bCalled);
    ASSERT_EQ(true, l3.Vars().update.bCalled);

    ASSERT_EQ(0, l1.Vars().update.nOrderCalled);
    ASSERT_EQ(1, l2.Vars().update.nOrderCalled);
    ASSERT_EQ(2, l3.Vars().update.nOrderCalled);

    ASSERT_EQ(true, l1.Vars().input.bCalled);
    ASSERT_EQ(true, l2.Vars().input.bCalled);
    ASSERT_EQ(true, l3.Vars().input.bCalled);

    ASSERT_EQ(0, l1.Vars().input.nOrderCalled);
    ASSERT_EQ(1, l2.Vars().input.nOrderCalled);
    ASSERT_EQ(2, l3.Vars().input.nOrderCalled);

    ASSERT_EQ(true, l1.Vars().draw.bCalled);
    ASSERT_EQ(true, l2.Vars().draw.bCalled);
    ASSERT_EQ(true, l3.Vars().draw.bCalled);

    ASSERT_EQ(0, l1.Vars().draw.nOrderCalled);
    ASSERT_EQ(1, l2.Vars().draw.nOrderCalled);
    ASSERT_EQ(2, l3.Vars().draw.nOrderCalled);
}


TEST(GameFramework, MaskingDisablesLayersBelow)
{
    int callCounter = 0;
    ScopedTestLayer l1{callCounter};
    ScopedTestLayer l2{callCounter};
    ScopedTestLayer l3{callCounter};

    l1.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);
    l2.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop | MasksDraw | MasksInput | MasksUpdate);
    l3.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);

    {
        ScopedGameFramework gf;
        GF_PushGameFrameworkLayer(&l1.layer);
        GF_PushGameFrameworkLayer(&l2.layer);
        GF_PushGameFrameworkLayer(&l3.layer);
        GF_EndFrame(nullptr, nullptr);
        GF_UpdateGameFramework(0.0f);
        ASSERT_EQ(2, callCounter);
        callCounter = 0;
        GF_InputGameFramework(nullptr);
        ASSERT_EQ(2, callCounter);
        callCounter = 0;
        GF_DrawGameFramework(nullptr);
        ASSERT_EQ(2, callCounter);
    }

    ASSERT_EQ(false, l1.Vars().update.bCalled);
    ASSERT_EQ(true, l2.Vars().update.bCalled);
    ASSERT_EQ(true, l3.Vars().update.bCalled);

    ASSERT_EQ(0, l2.Vars().update.nOrderCalled);
    ASSERT_EQ(1, l3.Vars().update.nOrderCalled);

    ASSERT_EQ(false, l1.Vars().input.bCalled);
    ASSERT_EQ(true, l2.Vars().input.bCalled);
    ASSERT_EQ(true, l3.Vars().input.bCalled);

    ASSERT_EQ(0, l2.Vars().input.nOrderCalled);
    ASSERT_EQ(1, l3.Vars().input.nOrderCalled);

    ASSERT_EQ(false, l1.Vars().draw.bCalled);
    ASSERT_EQ(true, l2.Vars().draw.bCalled);
    ASSERT_EQ(true, l3.Vars().draw.bCalled);

    ASSERT_EQ(0, l2.Vars().draw.nOrderCalled);
    ASSERT_EQ(1, l3.Vars().draw.nOrderCalled);

}

TEST(GameFramework, MaskingDisablesLayersBelowAndPoppingClears)
{
    int callCounter = 0;
    ScopedTestLayer l1{callCounter};
    ScopedTestLayer l2{callCounter};
    ScopedTestLayer l3{callCounter};

    l1.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn);
    l2.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | MasksDraw | MasksInput | MasksUpdate);
    l3.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn);

    {
        ScopedGameFramework gf;
        GF_PushGameFrameworkLayer(&l1.layer);
        GF_PushGameFrameworkLayer(&l2.layer);
        GF_PushGameFrameworkLayer(&l3.layer);
        GF_EndFrame(nullptr, nullptr);
        GF_UpdateGameFramework(0.0f);
        ASSERT_EQ(2, callCounter);
        callCounter = 0;
        GF_InputGameFramework(nullptr);
        ASSERT_EQ(2, callCounter);
        callCounter = 0;
        GF_DrawGameFramework(nullptr);
        ASSERT_EQ(2, callCounter);
        

        GF_PopGameFrameworkLayer();
        GF_PopGameFrameworkLayer();
        GF_PopGameFrameworkLayer();
        l2.layer.flags = l1.layer.flags; // clear mask bits
        GF_PushGameFrameworkLayer(&l1.layer);
        GF_PushGameFrameworkLayer(&l2.layer);
        GF_PushGameFrameworkLayer(&l3.layer);
        GF_EndFrame(nullptr, nullptr);

        callCounter = 0;
        GF_UpdateGameFramework(0.0f);
        ASSERT_EQ(3, callCounter);
        callCounter = 0;
        GF_InputGameFramework(nullptr);
        ASSERT_EQ(3, callCounter);
        callCounter = 0;
        GF_DrawGameFramework(nullptr);
        ASSERT_EQ(3, callCounter);
    }

    
}

TEST(GameFramework, WindowDimsChange)
{
    int callCounter = 0;
    ScopedTestLayer l1{callCounter};
    ScopedTestLayer l2{callCounter};
    ScopedTestLayer l3{callCounter};

    l1.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);
    l2.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop | MasksDraw | MasksInput | MasksUpdate);
    l3.layer.flags |= (EnableUpdateFn | EnableInputFn | EnableDrawFn | EnableOnPop);

    {
        ScopedGameFramework gf;
        GF_PushGameFrameworkLayer(&l1.layer);
        GF_PushGameFrameworkLayer(&l2.layer);
        GF_PushGameFrameworkLayer(&l3.layer);
        GF_EndFrame(nullptr, nullptr);
        GF_OnWindowDimsChanged(123, 321);
        ASSERT_EQ(l1.Vars().windowW, 123);
        ASSERT_EQ(l1.Vars().windowH, 321);
        ASSERT_EQ(l2.Vars().windowW, 123);
        ASSERT_EQ(l2.Vars().windowH, 321);
        ASSERT_EQ(l3.Vars().windowW, 123);
        ASSERT_EQ(l3.Vars().windowH, 321);
        GF_OnWindowDimsChanged(32, 54);
        ASSERT_EQ(l1.Vars().windowW, 32);
        ASSERT_EQ(l1.Vars().windowH, 54);
        ASSERT_EQ(l2.Vars().windowW, 32);
        ASSERT_EQ(l2.Vars().windowH, 54);
        ASSERT_EQ(l3.Vars().windowW, 32);
        ASSERT_EQ(l3.Vars().windowH, 54);
    }
}