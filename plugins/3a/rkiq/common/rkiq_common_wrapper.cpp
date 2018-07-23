#include <base/xcam_3a_description.h>
#include <xcam_std.h>

#include "aec.h"

using namespace XCam;

#define AIQ_CONTEXT_CAST(context)  ((XCamAERKiqContext*)(context))

class XCamAERKiqContext
{
public:
    XCamAERKiqContext ();
    ~XCamAERKiqContext ();

    void set_stats_buffer(AecStat_t* stats);
    AecStat_t* get_stats_buffer ();

public:
    bool _aec_initialized;    
    AecContext_t *_aec_context;
private:
    AecStat_t _aec_stats;
};

XCamAERKiqContext::XCamAERKiqContext ():
    _aec_initialized(false)
{
}

XCamAERKiqContext::~XCamAERKiqContext ()
{

}

void
XCamAERKiqContext::set_stats_buffer (AecStat_t* stats) {
    _aec_stats = *stats;
}

AecStat_t*
XCamAERKiqContext::get_stats_buffer () {
    return &_aec_stats;
}


static XCamReturn
xcam_create_context (XCam3AContext **context)
{
    XCAM_LOG_INFO ("xcam_create_context");
    XCAM_ASSERT (context);    
    XCamAERKiqContext *aiq_context = new XCamAERKiqContext ();
    *context = ((XCam3AContext*)(aiq_context));
    return XCAM_RETURN_NO_ERROR;

}

static XCamReturn
xcam_destroy_context (XCam3AContext *context)
{
    XCamAERKiqContext *aiq_context = AIQ_CONTEXT_CAST (context);
    delete aiq_context;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
xcam_set_stats (XCam3AContext *context, void *stats)
{
    XCamAERKiqContext *aiq_context = AIQ_CONTEXT_CAST (context);
    aiq_context->set_stats_buffer((AecStat_t*)stats);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
xcam_update_ae_params (XCam3AContext *context, void *params)
{
    XCamAERKiqContext *aiq_context = AIQ_CONTEXT_CAST (context);

    if (!aiq_context->_aec_initialized) {
        aiq_context->_aec_initialized = true;
        AecConfig_t *aecConfig = (AecConfig_t*)params;    
        int ret = AecInit(&aiq_context->_aec_context, (AecConfig_t*)params);
    } else {
        AecStop(aiq_context->_aec_context);
        int ret = AecUpdateConfig(aiq_context->_aec_context, (AecConfig_t*)params);
        AecStart(aiq_context->_aec_context);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
xcam_analyze_ae (XCam3AContext *context, XCamAeParam *params)
{
    XCamAERKiqContext *aiq_context = AIQ_CONTEXT_CAST (context);

    int ret = AecRun(aiq_context->_aec_context, aiq_context->get_stats_buffer(), NULL);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn

xcam_get_results (XCam3AContext *context, void* result)
{
    XCamAERKiqContext *aiq_context = AIQ_CONTEXT_CAST (context);

    AecGetResults(aiq_context->_aec_context, (AecResult_t*)result);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
xcam_combine_analyze_results (XCam3AContext *context, XCam3aResultHead *results[], uint32_t *res_count)
{
    return XCAM_RETURN_NO_ERROR;
}

static void
xcam_free_results (XCam3aResultHead *results[], uint32_t res_count)
{

}

XCAM_BEGIN_DECLARE

XCamAEDescription xcam_ae_desciption = {
    XCAM_HANDLER_AE,
    XCAM_VERSION,
    sizeof (XCamAEDescription),
    xcam_create_context,
    xcam_destroy_context,
    xcam_set_stats,
    xcam_update_ae_params,
    xcam_analyze_ae,
    xcam_get_results,
    xcam_combine_analyze_results,
    xcam_free_results
};

XCAM_END_DECLARE

