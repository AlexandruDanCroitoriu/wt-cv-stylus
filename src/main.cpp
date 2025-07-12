#include "000-Server/Server.h"
#include "001-App/App.h"

int main(int argc, char **argv)
{
 
    std::cout << "Starting Wt server..." << std::endl;

    Server server(argc, argv);

    server.run();

    return 0;
}

// try
//   {

//       Server server(argc, argv);
      
//       server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
//       server.addEntryPoint(
//           Wt::EntryPointType::Application,
//           [=](const Wt::WEnvironment &env)
//           {
//               return std::make_unique<App>(env);
//           },
//           "/");
//       // Session::configureAuth();

//       server.run();
//   }
//   catch (Wt::WServer::Exception &e)
//   {
//       std::cerr << e.what() << "\n";
//       return 1;
//   }
//   catch (std::exception &e)
//   {
//       std::cerr << "exception: " << e.what() << "\n";
//       return 1;
//   }