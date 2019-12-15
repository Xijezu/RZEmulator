/*
 *  Copyright (C) 2018-2018 NGemity <https://ngemity.org/>
 *  Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *  Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "World.h"

#if PLATFORM != PLATFORM_WINDOWS

#include <readline/readline.h>
#include <readline/history.h>

char *command_finder(const char *text, int32_t state)
{
    /*
    static size_t idx, len;
    const char* ret;
    std::vector<ChatCommand> const& cmd = ChatHandler::getCommandTable();

    if (!state)
    {
        idx = 0;
        len = strlen(text);
    }

    while (idx < cmd.size())
    {
        ret = cmd[idx].Name;
        if (!cmd[idx].AllowConsole)
        {
            ++idx;
            continue;
        }

        ++idx;
        //printf("Checking %s \n", cmd[idx].Name);
        if (strncmp(ret, text, len) == 0)
            return strdup(ret);
    }
    */
    return ((char *)NULL);
}

char **cli_completion(const char *text, int32_t start, int32_t /*end*/)
{
    char **matches = NULL;

    if (start)
        rl_bind_key('\t', rl_abort);
    else
        matches = rl_completion_matches((char *)text, &command_finder);
    return matches;
}

int32_t cli_hook_func()
{
    if (World::IsStopped())
        rl_done = 1;
    return 0;
}

#endif

void utf8print(void * /*arg*/, const char *str)
{
#if PLATFORM == PLATFORM_WINDOWS
    wchar_t wtemp_buf[6000];
    size_t wtemp_len = 6000-1;
    if (!Utf8toWStr(str, strlen(str), wtemp_buf, wtemp_len))
        return;

    wprintf(L"%s", wtemp_buf);
#else
    {
        printf("%s", str);
        fflush(stdout);
    }
#endif
}

void commandFinished(void *, bool /*success*/)
{
    printf("NG> ");
    fflush(stdout);
}

#if PLATFORM ==  PLATFORM_UNIX

// Non-blocking keypress detector, when return pressed, return 1, else always return 0
int32_t kb_hit_return()
{
    struct timeval tv;
    fd_set         fds;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

#endif

/// %Thread start
void CliThread()
{
    ///- Display the list of available CLI functions then beep
    //TC_LOG_INFO("server.worldserver", "");
#if PLATFORM != PLATFORM_WINDOWS
    rl_attempted_completion_function = cli_completion;
    rl_event_hook                    = cli_hook_func;
#endif

    // print32_t this here the first time
    // later it will be printed after command queue updates
    //printf("NG> ");

    ///- As long as the World is running (no World::m_stopEvent), get the command line and handle it
    while (!World::IsStopped())
    {
        fflush(stdout);

        char *command_str;             // = fgets(commandbuf, sizeof(commandbuf), stdin);

#if PLATFORM == PLATFORM_WINDOWS
        char commandbuf[256];
        command_str = fgets(commandbuf, sizeof(commandbuf), stdin);
#else
        command_str = readline("NG> ");
        rl_bind_key('\t', rl_complete);
#endif

        if (command_str != NULL)
        {
            for (int32_t x = 0; command_str[x]; ++x)
                if (command_str[x] == '\r' || command_str[x] == '\n')
                {
                    command_str[x] = 0;
                    break;
                }

            if (!*command_str)
            {
#if PLATFORM == PLATFORM_WINDOWS
                printf("NG> ");
#else
                free(command_str);
#endif
                continue;
            }

            std::string command;
            if (!consoleToUtf8(command_str, command))         // convert from console encoding to utf8
            {
#if PLATFORM == PLATFORM_WINDOWS
                printf("NG> ");
#else
                free(command_str);
#endif
                continue;
            }

            fflush(stdout);
            if (command == "quit")
                World::StopNow(SHUTDOWN_EXIT_CODE);
            //sWorld->QueueCliCommand(new CliCommandHolder(NULL, command.c_str(), &utf8print, &commandFinished));
#if PLATFORM != PLATFORM_WINDOWS
            add_history(command.c_str());
            free(command_str);
#endif
        }
        else if (feof(stdin))
        {
            World::StopNow(SHUTDOWN_EXIT_CODE);
        }
    }
}