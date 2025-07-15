#include "000-Server/Server.h"
#include "001-App/App.h"

// #include <Wt/WText.h>
// #include <Wt/WLineEdit.h>
// #include <Wt/WPushButton.h>

// class HelloApplication : public Wt::WApplication
// {
// public:
//     HelloApplication(const Wt::WEnvironment& env);

// private:
//     Wt::WLineEdit *nameEdit_;
//     Wt::WText *greeting_;
// };

// HelloApplication::HelloApplication(const Wt::WEnvironment& env)
//     : Wt::WApplication(env)
// {
//     setTitle("Hello world");

//     root()->addNew<Wt::WText>("Your name, please? ");
//     nameEdit_ = root()->addNew<Wt::WLineEdit>();
//     Wt::WPushButton *button = root()->addNew<Wt::WPushButton>("Greet me.");
//     root()->addNew<Wt::WBreak>();
//     greeting_ = root()->addNew<Wt::WText>();
//     auto greet = [this]{
//       greeting_->setText("Hello there, " + nameEdit_->text());
//     };
//     button->clicked().connect(greet);
// }

// int main(int argc, char **argv)
// {
//     return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
//       return std::make_unique<HelloApplication>(env);
//     });
// }


int main(int argc, char **argv)
{
 
    std::cout << "Starting Wt server..." << std::endl;

    Server server(argc, argv);

    server.run();

    return 0;
}


// int main(int argc, char **argv)
// {
 
//     std::cout << "Starting Wt server..." << std::endl;

//     Server server(argc, argv);

//     server.run();

//     return 0;
// }

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