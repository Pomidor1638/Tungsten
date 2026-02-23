

#include <iostream>
#include <iomanip>
#include <fstream>
#include "client/client.h"

std::string the_skull = R"(
                .:-+********+=:                 
             :-=++***#####%%#%##*+.             
           ::-=+***#####%%%%%%%%%%%#*.          
          .::=+***###%%%%%%%%%%%%%%%%#*:        
          ..:=***###%%%%%%%%%%%%%@%%%%##*.      
         ...:-+***##%%%####%%%@@@@%%%%%###-     
         ...:-=+***#%%%#%%%%%%@@@@@%%%%###*=    
         ...:-=****###%%%#%%%%%@@@@%%%%####*:   
        :...:-=+*###%%%%%%%%%%%%%@%%%%%##%#*+   
       ::...:-=+++**#%%%%%%%%%%%@%%%%%%%%%#**:  
       ::::::+++++++***#####%##*=**##%%%%%#**-  
       :-:-: .::..:::-******##*-.   .:-*%%%**-  
       ::              :+##+-:           :#*+*: 
       .                 *#*-.            .***: 
      .=                -##*:.            :**+: 
      :*=.              =*##*:            ***=- 
       **-::        :.  : :##%*:         =%#*=- 
      .=**+    .:-+=: .    -#%%%%*+-    -#%#*:: 
       .-+*****++=-:..      :*###*#%%%@%%%%#*-  
       .::::..    . :.       -**+:  :=***#%##*. 
        :=++:::    .-        :##***+==+***##**: 
       .:::-=+=:    -:  :.   =##******+=-=#*+:  
        .    .:.  .-:-::.*--*#*%***#*=.     ::  
              .:..*+-=-**##**#*#%%%*.   .-..:   
               .  #*=*=###%**######:    :+--    
                  *=:=.*+=*#*#*#*++:    :+*:    
                  -*:*:-*=+#*#=**#%#  ..=+*     
                        .   :-+=+=+*.:.-+*+     
                       . : .::+===:=::-***=     
                      .:::----+*******#***-     
                    .::==+******###**##***-     
                    .:-+++*##*+**-:*****+:      
                   .:--:-=**#####**#**+.        
                      .   .::::::-:
)";



void Init_API()
{
    if (SDL_Init(SDL_INIT_EVERYTHING))
    {
        throw std::runtime_error(std::string(__FUNCSIG__) + ": Can't Init SDL");
    }
}

std::ofstream log_file;

void Init_Logger()
{
    gLog.addStream(std::cout);

    log_file.open("log.log");
    if (log_file.is_open())
    {
        gLog.addStream(log_file);
    }
}

void Init_Hunk(std::size_t size)
{
    hunk.resize(size);
}


void Init(int argc, char* argv[])
{
    Init_API();
    Init_Logger();
    constexpr size_t HUNK_SIZE = (size_t(1) << 20);
    Init_Hunk(HUNK_SIZE);
}


void Quit()
{
    log_file.close();
    SDL_Quit();
}

void HandleError(std::string what)
{
    gLog << the_skull << std::endl;
    gLog << what << std::endl;
     
    ShowCheckboxError("Critical Error", "Trouble func", what);
}

int main(int argc, char* argv[])
{
    int code;
    try
    {
        Init(argc, argv);
        Client* client = hunk.allocate<Client, 1>(argc, argv);
        code = client->exec();
    }
    catch (const std::runtime_error& x)
    {
        HandleError(x.what());
        code = EXIT_FAILURE;
    }
    catch (...)
    {
        HandleError("Unexcepted Error");
        code = EXIT_FAILURE;
    }
    Quit();

    return code;
}