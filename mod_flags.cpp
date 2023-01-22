#include "server.hpp"

using namespace irc;

void	Server::_o(char sign, string _channel, string _nick, Client * _client)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
        std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (sign == '+')
            {
                chan->second->operators.insert(std::pair<string, Client *> (_nick, find_client(_nick)));
                chan->second->moderators.insert(std::pair<string, Client *> (_nick, find_client(_nick)));
                send_msg(find_client(_nick), ":" + _client->user_info() + " PRIVMSG " + _nick + " : You just got oper_privelage at " + _channel);
            }
            else if (sign == '-')
            {
                chan->second->operators.erase(_nick);
                send_msg(find_client(_nick), ":" + _client->user_info() + " PRIVMSG " + _nick + " : You just lost oper_privelage at " + _channel);
            }
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_i(char sign, string _channel, Client * _client)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
        std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (sign == '+')
                chan->second->modes += 'i';
            else if (sign == '-')
                chan->second->modes.erase(chan->second->modes.find('i'));
            send_msg(_client ,":Rαɠɳαɾöƙ MODE " + chan->second->modes + " " + sign + 'i');
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_m(char sign, string _channel, Client * _client)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
        std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (sign == '+')
                chan->second->modes += 'm';
            else if (sign == '-')
                chan->second->modes.erase(chan->second->modes.find('m'));
            send_msg(_client ,":Rαɠɳαɾöƙ MODE " + chan->second->modes + " " + sign + 'm');
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_v(char sign, string _channel, string _nick, Client * _client)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
        std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (sign == '+')
            {
                chan->second->moderators.insert(std::pair<string, Client *> (_nick, find_client(_nick)));
                send_msg(find_client(_nick), ":" + _client->user_info() + " PRIVMSG " + _nick + " : You just got permission to chat at " + _channel);
            }
            else if (sign == '-')
            {
                chan->second->moderators.erase(_nick);
                send_msg(find_client(_nick), ":" + _client->user_info() + " PRIVMSG " + _nick + " : You just lost permission to chat at " + _channel);
            }
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_k(char sign, string _channel, string _key, Client * _client)
{
    (void)sign;(void)_key;
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
         std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
            send_msg(_client, "467 * " + _channel + " :Channel key already set");
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_l(char sign, string _channel, string _limit, Client * _client)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
         std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (sign == '+')
            {
                if ((int)chan->second->members.size() < std::stoi(_limit))
                    chan->second->max_numbers = std::stoi(_limit);
            }
            else if (sign == '-')
                return;
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_t(char sign, string _channel, Client * _client)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
    if (chan != this->channels.end())
    {
         std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (sign == '+')
                chan->second->modes += 't';
            else if (sign == '-')
                chan->second->modes.erase(chan->second->modes.find('t'));
            send_msg(_client ,":Rαɠɳαɾöƙ MODE " + chan->second->modes + " " + sign + 't');
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}

void	Server::_b(char sign, string _channel, Client * _client, string _user)
{
    std::map<string, Channel *>::iterator chan = this->channels.find(_channel);
	if (_user.find('!') == string::npos || _user.find('@') == string::npos)
		send_msg(_client, ERR_NOSUCHNICK(_user));
    else if (chan != this->channels.end())
    {
         std::map<string, Client *>::iterator oper = chan->second->operators.find(_client->nick);
        if (oper != chan->second->operators.end())
        {
            if (_user == "")
            {
                for (std::map<string, bool>::iterator it = chan->second->bans.begin(); it != chan->second->bans.end(); it++)
                    send_msg(_client, "367 * " + _channel + " " + it->first);
                send_msg(_client, "368 * RPL_ENDOFBANLIST");
            }
			else
			{
				if (sign == '+')
				{
					chan->second->bans.insert(std::pair <string, bool> (_user, 0));
					if (_user == "*!*@*")
					{
						std::map<string, Client *>::iterator it = chan->second->members.begin();
						for (; it != chan->second->members.end(); it++)
							if (it->second->nick != _client->nick)
								this->leave_channels(it->second, chan->first);
					}
					else if (chan->second->members.find(_user.substr(0, _user.find('!'))) != chan->second->members.end())
						this->leave_channels(chan->second->members.find(_user.substr(0, _user.find('!')))->second, chan->first);
				}
				else if (sign == '-')
                    chan->second->bans.erase(_user);
			}
        }
        else
            send_msg(_client, ERR_CHANOPRIVSNEEDED(_channel));
    }
    else
        send_msg(_client, ERR_NOSUCHCHAN(_channel));
}
