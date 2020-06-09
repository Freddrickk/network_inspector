#include <Windows.h>

int ProcessGetBacktrace(uintptr_t rip, uintptr_t rsp, uintptr_t* backtrace, int nb_frame)
{
    CONTEXT ctx;
    PRUNTIME_FUNCTION runtime_functions;
    KNONVOLATILE_CONTEXT_POINTERS ctx_ptrs = {0};
    uintptr_t image_base, establisher_frame;
    void* handler_data;
    int frame_idx;

    if (rip == NULL || rsp == NULL || backtrace == NULL)
    {
        return -1;
    }

    RtlCaptureContext(&ctx);
    ctx.Rip = rip;
    ctx.Rsp = rsp;

    for (frame_idx = 0; frame_idx < nb_frame; frame_idx++)
    {
        runtime_functions = RtlLookupFunctionEntry(ctx.Rip, &image_base, NULL);
        if (runtime_functions != NULL)
        {
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, image_base, ctx.Rip, runtime_functions, &ctx, &handler_data, &establisher_frame, &ctx_ptrs);
        }
        else
        {
            ctx.Rip = *(uintptr_t*)ctx.Rsp;
            ctx.Rsp = ctx.Rsp + sizeof(uintptr_t);
        }

        if (ctx.Rip == NULL)
            break;
        backtrace[frame_idx] = ctx.Rip;
    }
    return frame_idx;
}
