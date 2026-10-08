#ifndef __CEF3_ClientHandler__
#define __CEF3_ClientHandler__

#include "../../extern/include/cef/cef_app.h"
#include "../../extern/include/cef/cef_client.h"

class ClientApp : public CefApp, public CefRenderProcessHandler, public CefBrowserProcessHandler {
public:
	ClientApp();

	CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
	{
		return this;
	}

	CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE
	{
		return this;
	}

	void OnWebKitInitialized() OVERRIDE;
	void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) OVERRIDE;

	IMPLEMENT_REFCOUNTING(ClientApp);
};

#endif /* defined(__CEF3_ClientHandler__) */
