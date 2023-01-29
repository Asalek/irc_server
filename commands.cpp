#include "server.hpp"
#include "client.hpp"
#include <algorithm>

using namespace irc;

void	Server::_NICK(string s_token, Client * client, string nick)
{
    int n = 0;
    if (s_token == "NICK\r\n" || s_token == "NICK\n" || (s_token == "NICK" && nick == "")) //hundle no nickname given // WE SHOULD HANDLE ONLY WHITE SPACES AFTER NICK
        send_msg(client, ERR_NONICKNAMEGIVEN);
    else if (s_token == "NICK") //set Nick_Name
    {
        client->nick = nick;
        for (size_t i = 0; i < this->clients.size(); i++)
        {
            if (this->clients[i]->nick == client->nick)
                n++;
            if (n > 1){
                string dup = "433 * " + client->nick + ":Nickname is already in use";
                send_msg(client, dup);
                client->nick = "";
                break ;
            }
        }
    }
}

void	Server::_USER(string s_token, Client * client, string user)
{
    if (s_token == "USER\r\n" || s_token == "USER\n" || (s_token == "USER" && user == "")) //hundle no nickname given // WE SHOULD HANDLE ONLY WHITE SPACES AFTER NICK
        send_msg(client, ERR_NEEDMOREPARAMS("USER"));
    else if (s_token == "USER") //set User_Name
    {
        if (client->username != "")
        {
            send_msg(client, ERR_ALREADYREGISTRED);
            return ;
        }
        if (this->params_calc(user) < 4)
        {
            send_msg(client, ERR_NEEDMOREPARAMS("USER"));
            return ;
        }
        int i = user.find(' ');
        client->username = user.substr(0, i);
    }
}

void	Server::_PASS(string s_token, Client * client, string pass)
{
    if (s_token == "PASS\r\n" || s_token == "PASS\n" || (s_token == "PASS" && pass == "")) //hundle no nickname given // WE SHOULD HANDLE ONLY WHITE SPACES AFTER NICK
        send_msg(client, ERR_NEEDMOREPARAMS("PASS"));
    else if (s_token == "PASS") //set Password
    {
        if (client->pass != "")
        {
            send_msg(client, ERR_ALREADYREGISTRED);
            return ;
        }
        if (pass != this->password)
        {
            send_msg(client, ERR_PASSWDMISMATCH);
            return ;
        }
		client->pass = pass;
    }
}

void    Server::_NOTICE(string s_token, Client * client, string msg)
{
    if (s_token == "NOTICE\r\n" || s_token == "NOTICE\n" || (s_token == "NOTICE" && msg == "")) //hundle no notice command given // WE SHOULD HANDLE ONLY WHITE SPACES AFTER NOTICE
        return ;
    else if (s_token == "NOTICE")
    {
        char * msg_token = strdup(const_cast<char *> (msg.c_str()));
		if (msg_token == NULL)
			return;
        char * nick_name = strtok_r(msg_token, " ",
            &msg_token);
        if (msg_token && nick_name)
        {
            Client * receiver = find_client(nick_name);
            if (receiver != nullptr)
            {
                string letter = ":" + client->user_info() + " PRIVMSG " + string(nick_name) + " : " + msg_token;
                send_msg(receiver, letter);
            }
        }
		free(nick_name);
    }
}

int		Server::_PRIVMSG(string s_token, Client * client, string msg)
{
	if (s_token == "PRIVMSG\r\n" || s_token == "PRIVMSG\n" || (s_token == "PRIVMSG" && msg == ""))
		return (send_msg(client, ERR_NORECIPIENT("PRIVMSG")), 1);
    else if (s_token == "PRIVMSG")
    {
	    std::vector<string> cls;

        char *client_nick;
        string	result;
        string	sms;

        client_nick = strtok(const_cast<char *> (msg.c_str()), ",");
        while (client_nick != NULL)
        {
            result = client_nick;
            trim_whiteSpaces(result);
            cls.push_back(result);
            client_nick = strtok(NULL, ",");
        }
        sms = *(cls.end() - 1);
        size_t start = sms.find_first_of(" ");
        if (start != string::npos)
        {
            *(cls.end() - 1) = sms.substr(0, start);
            sms = sms.substr(start + 1, sms.length() - 1);
        }
        else
            sms = "";
        string valid_cls = check_nickNAMEs(cls);
		string valid_chan = check_channNAMEs(cls);
		if (valid_cls != "" && valid_chan != "")
			return (send_msg(client, valid_chan), 1);
        if (sms == "")
            return (send_msg(client, ERR_NOTEXTTOSEND), 1);
        size_t find_pts = sms.find(':');
        if (find_pts == string::npos)
            sms.insert(0, ": ");
        else if (sms[find_pts + 1] != ' ')
            sms.insert(find_pts + 1, " ");
        if (sms.find("DCC") == string::npos) // direct client to client file transfer doesn't require space
			sms.insert(0, " ");
		if (valid_cls == "")
        	for (size_t i = 0; i < cls.size(); i++)
            	send_msg(this->find_client(cls[i]), ":" + client->user_info() + " PRIVMSG " + cls[i] + sms);
		else	//channels message
		{
			for (size_t i = 0; i < cls.size(); i++)
			{
				std::map<string, Channel *>::iterator target_chan = this->channels.find(cls[i]);
				if (target_chan->second->members.find(client->nick) == target_chan->second->members.end())
					return (send_msg(client, "442 * " + target_chan->first + " :You're not on that channel"), 0);
				for (std::map<string, Client *>::iterator it = target_chan->second->members.begin(); it != target_chan->second->members.end(); it++)
					if (it->first != client->nick)
					{
						if (target_chan->second->modes.find('m') != string::npos)
						{
							if (target_chan->second->moderators.find(client->nick) != target_chan->second->moderators.end()
								|| target_chan->second->operators.find(client->nick) != target_chan->second->operators.end())
            					send_msg(it->second, ":" + client->user_info() + " PRIVMSG " + cls[i] + sms);
						}
						else
							send_msg(it->second, ":" + client->user_info() + " PRIVMSG " + cls[i] + sms);
					}
			}
		}
	}
    return (0);
}

void    Server::_QUIT(string s_token, Client * client, int i, size_t index)
{
    if (s_token == "QUIT\r\n" || s_token == "QUIT\n" || s_token == "QUIT")
        this->disconnect(index, i, client->fd_socket);
}

void	Server::_JOIN(string s_token, Client * client, string chann)
{
	if (s_token == "JOIN\r\n" || s_token == "JOIN\n" || (s_token == "JOIN" && chann == ""))
	{
		send_msg(client, ERR_NEEDMOREPARAMS("JOIN"));
		return ;
	}
	else if (s_token == "JOIN")
	{
		string chns;
		string keys;
		char*	str;
		std::vector<string> v_channels;
		std::vector<string> v_keys;

		int i = find_spaceInBetween(chann);
		if (i)
		{
			chns = chann.substr(0, i);
			keys = chann.substr(i, chann.size() - i);
			str = strtok(const_cast<char *>(chns.c_str()), ",");//split channels with commas
			while (str != NULL)
			{
				string tmp = str;
				Server::trim_whiteSpaces(tmp);
				v_channels.push_back(tmp);
				str = strtok(NULL, ",");
			}
			str = strtok(const_cast<char *>(keys.c_str()), ",");//split keys with commas
			while (str != NULL)
			{
				string tmp = str;
				Server::trim_whiteSpaces(tmp);
				v_keys.push_back(tmp);
				str = strtok(NULL, ",");
			}
			if (v_keys.size() != v_channels.size())	//check that the same amount of channels and keys
			{
				send_msg(client, ERR_NEEDMOREPARAMS("JOIN"));
				return ;
			}
			for (size_t i = 0; i < v_keys.size(); i++)
			{
				if (this->channels.find(v_channels[i]) == this->channels.end())
					this->channels.insert(std::pair<string, Channel *>(v_channels[i], new Channel(v_channels[i], v_keys[i])));
				this->channels.find(v_channels[i])->second->joinChannel(v_channels[i], v_keys[i], client);
			}
		}
		else
			send_msg(client, ERR_NEEDMOREPARAMS("JOIN"));	//in case of JOIN failed
	}
}

void 	Server::_PART(string s_token, Client * client, string channs)
{
	if (s_token == "PART\r\n" || s_token == "PART\n" || (s_token == "PART" && channs == ""))
	{
		send_msg(client, ERR_NEEDMOREPARAMS("PART"));
		return ;
	}
	else if (s_token == "PART")
	{
		char *str = strtok(const_cast<char *>(channs.c_str()), ",");//split channels with commas
		string tmp = str;
		while (str != NULL)
		{
			tmp = str;
			size_t pos = tmp.find(':'); // revolution irc ':' requirement removal
			if (pos != string::npos)
				tmp.erase(pos);
			Server::trim_whiteSpaces(tmp);
			this->leave_channels(client, tmp);
			str = strtok(NULL, ",");
		}
	}
}

void 	Server::_KICK(string s_token, Client * client, string res)
{
	if (s_token == "KICK\r\n" || s_token == "KICK\n" || (s_token == "KICK" && res == ""))
	{
		send_msg(client, ERR_NEEDMOREPARAMS("KICK"));
		return ;
	}
	else if (s_token == "KICK")
	{
		string chns;
		string users;
		char*	str;
		std::vector<string> v_channels;
		std::vector<string> v_users;
		int i = find_spaceInBetween(res);
		if (i)
		{
			chns = res.substr(0, i);
			users = res.substr(i, res.size() - i);
			str = strtok(const_cast<char *>(chns.c_str()), ",");//split channel with commas
			std::map<string, Channel *>::iterator it;

			while (str != NULL)
			{
				string tmp = str;
				Server::trim_whiteSpaces(tmp);
				v_channels.push_back(tmp);
				str = strtok(NULL, ",");
			}
			str = strtok(const_cast<char *>(users.c_str()), ",");//split users with commas
			while (str != NULL)
			{
				string tmp = str;
				Server::trim_whiteSpaces(tmp);
				v_users.push_back(tmp);
				str = strtok(NULL, ",");
			}
			if (v_channels.size() != v_users.size())
			{
				send_msg(client, ERR_NEEDMOREPARAMS("KICK"));
				return ;
			}
			for (size_t i = 0; i < v_channels.size(); i++)
			{
				it = this->channels.find(v_channels[i]);
				if (it == this->channels.end() || this->channels.size() == 0)	//channel not exist
					send_msg(client, "403 * " + v_channels[i] + " :No such channel");
				else if (it->second->operators.find(client->nick) == it->second->operators.end()) //user not operator
					send_msg(client, "482 * " + it->first + " :You're not channel operator");
				else if (it->second->members.find(v_users[i]) == it->second->members.end())//kicked user not exist
					send_msg(client, "442 * " + v_users[i] + " : is not on that channel");
				else if (it->second->members.find(client->nick) == it->second->members.end()) //the kicker not on that channel
					send_msg(client, "442 * " + client->nick + " : not on channel");
				else
				{
					for (std::map<string, Client*>::iterator ii = it->second->members.begin(); ii != it->second->members.end(); ii++)
					{
						string msg = ":" + it->second->members.find(v_users[i])->first + " PART " + it->first + "\r\n";
						send_msg(ii->second, msg);
					}
					it->second->members.erase(v_users[i]);
					it->second->operators.erase(v_users[i]);
					it->second->moderators.erase(v_users[i]);
				}
			}
		}
	}
}

int 	Server::_MODE(string s_token, Client * client, string params)
{
	if (s_token == "MODE\r\n" || s_token == "MODE\n" || (s_token == "MODE" && params == ""))
		send_msg(client, ERR_NEEDMOREPARAMS("MODE"));
	else if (s_token == "MODE")
	{
		size_t params_nbr = params_calc(params);
		if (params_nbr < 2)
			return (send_msg(client, ERR_NEEDMOREPARAMS("MODE")), 1);
		char* part_param = strtok(const_cast<char *>(params.c_str()), " ");
		std::vector<string> vec_params;
		while (part_param)
		{
			vec_params.push_back(part_param);
			part_param = strtok(NULL, " ");
		}
		if (vec_params[1] == "-o" || vec_params[1] == "+o")
		{
			if (params_nbr < 3)
				return (send_msg(client, ERR_NEEDMOREPARAMS("MODE")), 1);
			this->_o(vec_params[1][0], vec_params[0], vec_params[2], client);
		}
		else if (vec_params[1] == "-i" || vec_params[1] == "+i")
			this->_i(vec_params[1][0], vec_params[0], client);
		else if (vec_params[1] == "-m" || vec_params[1] == "+m")
			this->_m(vec_params[1][0], vec_params[0], client);
		else if (vec_params[1] == "-v" || vec_params[1] == "+v")
		{
			if (params_nbr < 3)
				return (send_msg(client, ERR_NEEDMOREPARAMS("MODE")), 1);
			this->_v(vec_params[1][0], vec_params[0], vec_params[2], client);
		}
		else if (vec_params[1] == "-k" || vec_params[1] == "+k")
		{
			if (params_nbr < 3)
				return (send_msg(client, ERR_NEEDMOREPARAMS("MODE")), 1);
			this->_k(vec_params[1][0], vec_params[0], vec_params[2], client);
		}
		else if (vec_params[1] == "-l" || vec_params[1] == "+l")
		{
			if (params_nbr < 3)
				return (send_msg(client, ERR_NEEDMOREPARAMS("MODE")), 1);
			this->_l(vec_params[1][0], vec_params[0], vec_params[2], client);
		}
		else if (vec_params[1] == "-t" || vec_params[1] == "+t")
			this->_t(vec_params[1][0], vec_params[0], client);
		else if (vec_params[1] == "-b" || vec_params[1] == "+b")
		{
			if (params_nbr == 2)
				this->_b(vec_params[1][0], vec_params[0], client);
			else
				this->_b(vec_params[1][0], vec_params[0], client, vec_params[2]);
		}
		else
			send_msg(client, "501 * :Unknown MODE flag");
	}
	return 0;
}

int 	Server::_INVITE(string s_token, Client * client, string invited)
{
	std::map<string, Channel *>::iterator map_iter;

	if (s_token == "INVITE\r\n" || s_token == "INVITE\n" || (s_token == "INVITE" && invited == ""))
		return (send_msg(client, ERR_NEEDMOREPARAMS("INVITE")), 0);
	else if (s_token == "INVITE")
	{
		char *nick, *chan;
		string nickname;
		string channel;
		nick = strtok(const_cast<char *>(invited.c_str()), " ");
		chan = strtok(NULL, " ");
		if (!nick || !chan)
			return (send_msg(client, ERR_NEEDMOREPARAMS("_INVITE")), 0);
		nickname = nick;
		channel = chan;
		bool founded = false;
		size_t i = 0;
		for (; i < this->clients.size(); i++)
		{
			if (nickname == this->clients[i]->nick)
			{
				founded = true;
				break ;
			}
		}
		if (founded == false)	//no such client
			return (send_msg(client, "401 * " + nickname + " :No such nick"), 0);
		map_iter = this->channels.find(channel);
		if (map_iter == this->channels.end())
			return (send_msg(client, "403 * " + channel + " :No such channel"), 0);
		if (map_iter->second->operators.find(client->nick) == map_iter->second->operators.end())
			return (send_msg(client, "482 * " + channel + " :You're not channel operator"), 0);
		if (map_iter->second->members.find(this->clients[i]->nick) != map_iter->second->members.end())
			return (send_msg(client, "443 * " + this->clients[i]->nick + " " + channel + " :is already on channel"), 0);
		for (std::map<string, Client *>::iterator it = map_iter->second->members.begin(); it != map_iter->second->members.end(); it++)
			send_msg(it->second, "341 * " + channel + " " + nickname + "");
		send_msg(clients[i], "341 * " + clients[i]->nick + " " + channel);
		map_iter->second->invited_clients.insert(std::pair<string, int> (nickname, 1));
	}
	return 0;
}