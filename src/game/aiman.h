// server-side ai manager
// note that server does not handle actual bot logic,
// which is offloaded to the clients with the best connection
namespace aiman
{
    bool dorefresh = false, botbalance = true;
    VARN(serverbotlimit, botlimit, 0, 8, MAXBOTS);
    VAR(serverbotbalance, 0, 1, 1);

    void calcteams(vector<teamscore> &teams)
    {
        for(int i = 0; i < clients.length(); i++)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==ClientState_Spectator || !VALID_TEAM(ci->team))
            {
                continue;
            }
            teamscore *t = NULL;
            for(int j = 0; j < teams.length(); j++)
            {
                if(teams[j].team == ci->team)
                {
                    t = &teams[j];
                    break;
                }
            }
            if(t)
            {
                t->score++;
            }
            else
            {
                teams.add(teamscore(ci->team, 1));
            }
        }
        teams.sort(teamscore::compare);
        if(teams.length() < MAXTEAMS)
        {
            for(int i = 0; i < MAXTEAMS; ++i)
            {
                if(teams.htfind(1+i) < 0)
                {
                    teams.add(teamscore(1+i, 0));
                }
            }
        }
    }

    void balanceteams()
    {
        vector<teamscore> teams;
        calcteams(teams);
        vector<clientinfo *> reassign;
        for(int i = 0; i < bots.length(); i++)
        {
            if(bots[i])
            {
                reassign.add(bots[i]);
            }
        }
        while(reassign.length() && teams.length() && teams[0].score > teams.last().score + 1)
        {
            teamscore &t = teams.last();
            clientinfo *bot = NULL;
            for(int i = 0; i < reassign.length(); i++)
            {
                if(reassign[i] && reassign[i]->team != teams[0].team)
                {
                    bot = reassign.removeunordered(i);
                    teams[0].score--;
                    t.score++;
                    for(int j = teams.length() - 2; j >= 0; j--)
                    {
                        if(teams[j].score >= teams[j+1].score)
                        {
                            break;
                        }
                        swap(teams[j], teams[j+1]);
                    }
                    break;
                }
            }
            if(bot)
            {
                if(smode && bot->state.state==ClientState_Alive)
                {
                    smode->changeteam(bot, bot->team, t.team);
                }
                bot->team = t.team;
                sendf(-1, 1, "riiii", NetMsg_SetTeam, bot->clientnum, bot->team, 0);
            }
            else
            {
                teams.remove(0, 1);
            }
        }
    }

    int chooseteam()
    {
        vector<teamscore> teams;
        calcteams(teams);
        return teams.length() ? teams.last().team : 0;
    }

    static inline bool validaiclient(clientinfo *ci)
    {
        return ci->clientnum >= 0 && ci->state.aitype == AI_None && (ci->state.state!=ClientState_Spectator || ci->local || (ci->privilege && !ci->warned));
    }

    clientinfo *findaiclient(clientinfo *exclude = NULL)
    {
        clientinfo *least = NULL;
        for(int i = 0; i < clients.length(); i++)
        {
            clientinfo *ci = clients[i];
            if(!validaiclient(ci) || ci==exclude)
            {
                continue;
            }
            if(!least || ci->bots.length() < least->bots.length())
            {
                least = ci;
            }
        }
        return least;
    }

    bool addai(int skill, int limit)
    {
        int numai = 0, cn = -1, maxai = limit >= 0 ? min(limit, MAXBOTS) : MAXBOTS;
        for(int i = 0; i < bots.length(); i++)
        {
            clientinfo *ci = bots[i];
            if(!ci || ci->ownernum < 0) { if(cn < 0) cn = i; continue; }
            numai++;
        }
        if(numai >= maxai)
        {
            return false;
        }
        if(bots.inrange(cn))
        {
            clientinfo *ci = bots[cn];
            if(ci)
            { // reuse a slot that was going to removed

                clientinfo *owner = findaiclient();
                ci->ownernum = owner ? owner->clientnum : -1;
                if(owner) owner->bots.add(ci);
                ci->aireinit = 2;
                dorefresh = true;
                return true;
            }
        }
        else { cn = bots.length(); bots.add(NULL); }
        int team = modecheck(gamemode, Mode_Team) ? chooseteam() : 0;
        if(!bots[cn])
        {
            bots[cn] = new clientinfo;
        }
        clientinfo *ci = bots[cn];
        ci->clientnum = MAXCLIENTS + cn;
        ci->state.aitype = AI_Bot;
        clientinfo *owner = findaiclient();
        ci->ownernum = owner ? owner->clientnum : -1;
        if(owner)
        {
            owner->bots.add(ci);
        }
        ci->state.skill = skill <= 0 ? RANDOM_INT(50) + 51 : clamp(skill, 1, 101);
        clients.add(ci);
        ci->state.lasttimeplayed = lastmillis;
        copystring(ci->name, "bot", MAXNAMELEN+1);
        ci->state.state = ClientState_Dead;
        ci->team = team;
        ci->playermodel = RANDOM_INT(128);
        ci->playercolor = RANDOM_INT(0x8000);
        ci->aireinit = 2;
        ci->connected = true;
        dorefresh = true;
        return true;
    }

    void deleteai(clientinfo *ci)
    {
        int cn = ci->clientnum - MAXCLIENTS;
        if(!bots.inrange(cn))
        {
            return;
        }
        if(ci->ownernum >= 0 && !ci->aireinit && smode)
        {
            smode->leavegame(ci, true);
        }
        sendf(-1, 1, "ri2", NetMsg_ClientDiscon, ci->clientnum);
        clientinfo *owner = (clientinfo *)getclientinfo(ci->ownernum);
        if(owner)
        {
            owner->bots.removeobj(ci);
        }
        clients.removeobj(ci);
        DELETEP(bots[cn]);
        dorefresh = true;
    }

    bool deleteai()
    {
        for(int i = bots.length(); --i >=0;) //note reverse iteration
        {
            if(bots[i] && bots[i]->ownernum >= 0)
            {
                deleteai(bots[i]);
                return true;
            }
        }
        return false;
    }

    void reinitai(clientinfo *ci)
    {
        if(ci->ownernum < 0) deleteai(ci);
        else if(ci->aireinit >= 1)
        {
            sendf(-1, 1, "ri8s", NetMsg_InitAI, ci->clientnum, ci->ownernum, ci->state.aitype, ci->state.skill, ci->playermodel, ci->playercolor, ci->team, ci->name);
            if(ci->aireinit == 2)
            {
                ci->reassign();
                if(ci->state.state==ClientState_Alive)
                {
                    sendspawn(ci);
                }
                else
                {
                    sendresume(ci);
                }
            }
            ci->aireinit = 0;
        }
    }

    void shiftai(clientinfo *ci, clientinfo *owner = NULL)
    {
        if(ci->ownernum >= 0 && !ci->aireinit && smode)
        {
            smode->leavegame(ci, true);
        }
        clientinfo *prevowner = (clientinfo *)getclientinfo(ci->ownernum);
        if(prevowner)
        {
            prevowner->bots.removeobj(ci);
        }
        if(!owner)
        {
            ci->aireinit = 0;
            ci->ownernum = -1;
        }
        else if(ci->ownernum != owner->clientnum)
        {
            ci->aireinit = 2;
            ci->ownernum = owner->clientnum;
            owner->bots.add(ci);
        }
        dorefresh = true;
    }

    void removeai(clientinfo *ci)
    { // either schedules a removal, or someone else to assign to

        for(int i = ci->bots.length(); --i >=0;) //note reverse iteration
        {
            shiftai(ci->bots[i], findaiclient(ci));
        }
    }

    bool reassignai()
    {
        clientinfo *hi = NULL, *lo = NULL;
        for(int i = 0; i < clients.length(); i++)
        {
            clientinfo *ci = clients[i];
            if(!validaiclient(ci)) continue;
            if(!lo || ci->bots.length() < lo->bots.length())
            {
                lo = ci;
            }
            if(!hi || ci->bots.length() > hi->bots.length())
            {
                hi = ci;
            }
        }
        if(hi && lo && hi->bots.length() - lo->bots.length() > 1)
        {
            for(int i = hi->bots.length(); --i >=0;) //note reverse iteration
            {
                shiftai(hi->bots[i], lo);
                return true;
            }
        }
        return false;
    }


    void checksetup()
    {
        if(modecheck(gamemode, Mode_Team) && botbalance)
        {
            balanceteams();
        }
        for(int i = bots.length(); --i >=0;) //note reverse iteration
        {
            if(bots[i])
            {
                reinitai(bots[i]);
            }
        }
    }

    void clearai()
    { // clear and remove all ai immediately
        for(int i = bots.length(); --i >=0;) //note reverse iteration
        {
            if(bots[i])
            {
                deleteai(bots[i]);
            }
        }
    }

    void checkai()
    {
        if(!dorefresh)
        {
            return;
        }
        dorefresh = false;
        if(!modecheck(gamemode, Mode_Bot) && numclients(-1, false, true))
        {
            checksetup();
        }
        else
        {
            clearai();
        }
    }

    void reqadd(clientinfo *ci, int skill)
    {
        if(!ci->local && !ci->privilege)
        {
            return;
        }
        if(!addai(skill, !ci->local && ci->privilege < Priv_Admin ? botlimit : -1))
        {
            sendf(ci->clientnum, 1, "ris", NetMsg_ServerMsg, "failed to create or assign bot");
        }
    }

    void reqdel(clientinfo *ci)
    {
        if(!ci->local && !ci->privilege)
        {
            return;
        }
        if(!deleteai())
        {
            sendf(ci->clientnum, 1, "ris", NetMsg_ServerMsg, "failed to remove any bots");
        }
    }

    void setbotlimit(clientinfo *ci, int limit)
    {
        if(ci && !ci->local && ci->privilege < Priv_Admin)
        {
            return;
        }
        botlimit = clamp(limit, 0, MAXBOTS);
        dorefresh = true;
        DEF_FORMAT_STRING(msg, "bot limit is now %d", botlimit);
        sendservmsg(msg);
    }

    void setbotbalance(clientinfo *ci, bool balance)
    {
        if(ci && !ci->local && !ci->privilege)
        {
            return;
        }
        botbalance = balance ? 1 : 0;
        dorefresh = true;
        DEF_FORMAT_STRING(msg, "bot team balancing is now %s", botbalance ? "enabled" : "disabled");
        sendservmsg(msg);
    }


    void changemap()
    {
        dorefresh = true;
        for(int i = 0; i < clients.length(); i++)
        {
            if(clients[i]->local || clients[i]->privilege)
            {
                return;
            }
        }
        if(botbalance != (serverbotbalance != 0))
        {
            setbotbalance(NULL, serverbotbalance != 0);
        }
    }

    void addclient(clientinfo *ci)
    {
        if(ci->state.aitype == AI_None)
        {
            dorefresh = true;
        }
    }

    void changeteam(clientinfo *ci)
    {
        if(ci->state.aitype == AI_None)
        {
            dorefresh = true;
        }
    }
}
