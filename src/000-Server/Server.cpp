#define WTHTTP_CONFIGURATION "../wt_config.xml"

#include "000-Server/Server.h"
#include "001-App/App.h"
#include <Wt/WSslInfo.h>
#include <csignal>
#include <iostream>

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/Auth/FacebookService.h>
#include <Wt/Auth/Mfa/TotpProcess.h>

// #define WTHTTP_CONFIGURATION "../wt_config.xml"

// Define static members
Wt::Auth::AuthService Server::authService;
Wt::Auth::PasswordService Server::passwordService(Server::authService);
std::vector<std::unique_ptr<Wt::Auth::OAuthService>> Server::oAuthServices;

Server::Server(int argc, char **argv)
    : Wt::WServer(argc, argv),
        argc_(argc),
        argv_(argv)
{
    setServerConfiguration(argc_, argv_, WTHTTP_CONFIGURATION);
    configureAuth();
    
    std::cout << "Application arguments:" << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    }
    std::cout << "\n\n WTHTTP_CONFIGURATION: " << WTHTTP_CONFIGURATION << "\n\n";
    std::cout << "\n\n WT_CONFIG_XML: " << WT_CONFIG_XML << "\n\n";
    addEntryPoint(Wt::EntryPointType::Application, [](const Wt::WEnvironment& env) {
        {
            // std::cout << "\n\n";
            // std::cout << "env.accept():                         <" << env.accept() << ">\n";
            // std::cout << "env.agentIsChrome():                  <" << (env.agentIsChrome() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsGecko():                   <" << (env.agentIsGecko() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsIE():                      <" << (env.agentIsIE() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsIEMobile():                <" << (env.agentIsIEMobile() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsMobileWebKit():            <" << (env.agentIsMobileWebKit() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsOpera():                   <" << (env.agentIsOpera() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsSafari():                  <" << (env.agentIsSafari() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsSpiderBot():                <" << (env.agentIsSpiderBot() ? "true" : "false") << ">\n";
            // std::cout << "env.agentIsWebKit():                  <" << (env.agentIsWebKit() ? "true" : "false") << ">\n";
            // std::cout << "env.ajax():                           <" << (env.ajax() ? "true" : "false") << ">\n";
            // std::cout << "env.clientAddress():                   <" << env.clientAddress() << ">\n";
            // std::cout << "env.contentType():                    <";
            // switch (env.contentType()) {
            //         case Wt::HtmlContentType::XHTML1:
            //         std::cout << "XHTML1";
            //         break;
            //     case Wt::HtmlContentType::HTML4:
            //         std::cout << "HTML4";
            //         break;
            //     case Wt::HtmlContentType::HTML5:
            //         std::cout << "HTML5";
            //         break;
            //     default:
            //         std::cout << "Unknown";
            //         break;
            // }
            // std::cout << ">\n";
            // const auto& cookies = env.cookies();
            // std::cout << "env.cookies():                        <";
            // for (const auto& [name, value] : cookies) {
            //     std::cout << name << "=" << value << "; ";
            // }
            // std::cout << ">\n";
            // std::cout << "env.deploymentPath():                 <" << env.deploymentPath() << ">\n";
            // std::cout << "env.dpiScale():                       <" << env.dpiScale() << ">\n";
            // std::cout << "env.headerValue(\"Accept-Language\"):   <" << env.headerValue("Accept-Language") << ">\n";
            // std::cout << "env.hostName():                       <" << env.hostName() << ">\n";
            // std::cout << "env.internalPath():                   <" << env.internalPath() << ">\n";
            // std::cout << "env.internalPathUsingFragments():     <" << (env.internalPathUsingFragments() ? "true" : "false") << ">\n";
            // std::cout << "env.javaScript(): support             <" << (env.javaScript() ? "true" : "false") << ">\n";
            // std::cout << "env.libraryVersion():                 <" << env.libraryVersion() << ">\n";
            // std::cout << "env.locale().name():                  <" << env.locale().name() << ">\n";
            // std::cout << "env.referer():                        <" << env.referer() << ">\n";
            // std::cout << "env.screenHeight():                   <" << env.screenHeight() << ">\n";
            // std::cout << "env.screenWidth():                    <" << env.screenWidth() << ">\n";
            // std::cout << "env.serverAdmin():                    <" << env.serverAdmin() << ">\n";
            // std::cout << "env.serverSignature():                <" << env.serverSignature() << ">\n";
            // std::cout << "env.serverSoftware():                 <" << env.serverSoftware() << ">\n";
            // std::cout << "env.supportsCookies():                <" << (env.supportsCookies() ? "true" : "false") << ">\n";
            // std::cout << "env.timeZoneName():                   <" << env.timeZoneName() << ">\n";
            // std::cout << "env.timeZoneOffset()                  <" << std::to_string(env.timeZoneOffset().count()) << ">\n";
            // std::cout << "env.userAgent():                      <" << env.userAgent() << ">\n";
            // std::cout << "env.webGL(): support                  <" << (env.webGL() ? "true" : "false") << ">\n";
            // std::cout << "\n";
        }
        return std::make_unique<App>(env);
    }, "/");

    // run();
}

int Server::run()
{
 
   
    try {
        if (start()) {
            int sig = WServer::waitForShutdown();
            
            std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
            stop();

            if (sig == SIGHUP)
                restart(argc_, argv_, environ);
        }
    } catch (WServer::Exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

void Server::configureAuth()
{
    authService.setAuthTokensEnabled(true, "logincookie");
    authService.setEmailVerificationEnabled(false);
    authService.setEmailVerificationRequired(false);
    authService.setIdentityPolicy(Wt::Auth::IdentityPolicy::LoginName);
    
    // authService.setMfaProvider(Wt::Auth::Identity::MultiFactor);
    // authService.setMfaRequired(true);
    // authService.setMfaThrottleEnabled(true);

    auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(12));
    passwordService.setVerifier(std::move(verifier));
    passwordService.setPasswordThrottle(std::make_unique<Wt::Auth::AuthThrottle>());
    passwordService.setStrengthValidator(std::make_unique<Wt::Auth::PasswordStrengthValidator>());

    if (Wt::Auth::GoogleService::configured()) {
        oAuthServices.push_back(std::make_unique<Wt::Auth::GoogleService>(authService));
    }
    if (Wt::Auth::FacebookService::configured()) {
        oAuthServices.push_back(std::make_unique<Wt::Auth::FacebookService>(authService));
    }
    for (const auto& oAuthService : oAuthServices) {
        oAuthService->generateRedirectEndpoint();
    }
}
