#include "StdAfx.h"
#include "CefClientApp.h"

#include "CefClientHandler.h"
#include "CefClientV8ExtensionHandler.h"
#include "../../extern/include/cef/wrapper/cef_helpers.h"

ClientApp::ClientApp()
{
}

void ClientApp::OnWebKitInitialized()
{
	std::string app_code =
		"var app;"
		"if (!app)"
		"    app = {};"
		"(function() {"
		"    app.ChangeTextInJS = function(text) {"
		"        native function ChangeTextInJS();"
		"        return ChangeTextInJS(text);"
		"    };"
		"})();";

	CefRegisterExtension("v8/app", app_code, new ClientV8ExtensionHandler(this));
}

void ClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-web-security");
	command_line->AppendSwitch("allow-running-insecure-content");
	command_line->AppendSwitch("ignore-certificate-errors");
	command_line->AppendSwitch("disable-gpu");
	command_line->AppendSwitch("disable-gpu-compositing");
}
