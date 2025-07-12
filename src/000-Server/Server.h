#pragma once
#include <Wt/WServer.h>

class Server : public Wt::WServer
{
    public:
        Server(int argc, char **argv);
        int run();

        // Auth services as static members
        static Wt::Auth::AuthService authService;
        static Wt::Auth::PasswordService passwordService;
        static std::vector<std::unique_ptr<Wt::Auth::OAuthService>> oAuthServices;

    private:
        int argc_;
        char **argv_;

        void configureAuth();
};