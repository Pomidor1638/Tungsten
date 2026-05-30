

#include <iostream>
#include <fstream>

#include "client/client.h"
#include "common/memory/memory.h"
#include "utils/logger/logger.h"
#include "utils/system/system.h"

char the_skull[] = 
R"(
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
    tungsten::logger::gLog.addStream(std::cout);
    log_file.open("log.log");
    if (log_file.is_open())
    {
        tungsten::logger::gLog.addStream(log_file);
    }
}

void Init_Zone(std::size_t size)
{
    tungsten::memory::permanent_zone.resize(size);

    //tungsten::memory::permanent_zone;
    //tungsten::memory::level_zone;
    //tungsten::memory::frame_zone;
    //tungsten::memory::scratch_zone;
}


void Init(int argc, char* argv[])
{
    Init_API();
    Init_Logger();
    constexpr size_t HUNK_SIZE = (1ULL << 16);
    Init_Zone(HUNK_SIZE);
}

void Quit()
{
    log_file.close();
    SDL_Quit();
}

void HandleError(std::string what)
{
    tungsten::logger::gLog << the_skull << std::endl;
    tungsten::logger::gLog << what      << std::endl;
     
    ShowCheckboxError("Critical Error", "Trouble func", what);
}

int main(int argc, char* argv[])
{
    int code;
    try
    {
        Init(argc, argv);
        auto* client = tungsten::memory::permanent_zone.allocate<tungsten::client::Client, 1>(argc, argv);
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
