#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "Network/Server.h"

class CGame {
    public:
        /*** Implementacja singletona ***/
        static CGame& Get() {
            static CGame game;
            return game;
        }

        /*** Metody glowne ***/
        void Init();
        void Start();
        void End();

        bool IsEnd() { return m_end; }

        /*** Metody dostepowe ***/
        Network::CServer& Server() { return *server; }

    private:
        /*** Metody prywatne ***/
        CGame();
        ~CGame();

        /*** Obiekty podprogramow ***/
        Network::CServer* server;

        /*** Zmienne ***/
        bool        m_end;
};

#endif // GAME_H_INCLUDED
