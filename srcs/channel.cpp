/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salec <salec@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 12:43:52 by salec             #+#    #+#             */
/*   Updated: 2020/12/07 20:29:02 by gbright          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "channel.hpp"
#include "tools.hpp"

std::string	get_safe_postfix(void)
{
	time_t time = ft_getcurrenttime();
	std::string	postfix;
	int			index;

	index = 0;
	while (time > 0)
	{
		index = time % 36;
		time = time - time % 36;
		if (index < 26)
			postfix += static_cast<char>('A' + index);
		else if (index < 35)
			postfix += static_cast<char>('0' + index - 25);
		else
			postfix += "0";
	}
	return postfix;
}

channel_flags::channel_flags(void) : _anonymous(false), _invite_only(false),
	_moderated(false), _no_messages_outside(false), _quiet(false), _private(false),
	_secret(false), _reop(false), _topic_settable_by_chop(false), _key(""),
	_limit_of_users(1024) {}

channel_flags::~channel_flags(void) {}

client_flags::client_flags(bool Operator, bool oper, bool voice) :
	_Operator(Operator), _operator(oper), _voice(voice) {}

client_flags::client_flags(bool *mode) : _Operator(mode[0]),
	_operator(mode[1]), _voice(mode[2]) {}

client_flags::client_flags(client_flags const &obj)
{
	_Operator = obj._Operator;
	_operator = obj._operator;
	_voice = obj._voice;
}

client_flags	&client_flags::operator=(client_flags const &obj)
{
	_Operator = obj._Operator;
	_operator = obj._operator;
	_voice = obj._voice;
	return *this;
}

client_flags::client_flags(void) : _Operator(0), _operator(0), _voice(0) {}

Channel::Channel(std::string const &name, Client *client) : _name(name), _blocked(0),
	_creation_time(ft_getcurrenttime())
{
	if (name.size() > 0 && name[0] == '#')
		_clients[client] = client_flags(1, 1, 0);
	else if (name.size() > 0 && name[0] == '+')
		_clients[client];
	else if (name.size() > 0 && name[0] == '!')
	{
		_safe_postfix = get_safe_postfix();
		_clients[client] = client_flags(1, 1, 0);
	}
}

Channel::Channel(std::string const &name, Client *client, std::string const &modes) :
	_name(name), _blocked(0), _creation_time(ft_getcurrenttime())
{
	bool	bmodes[3];

	bmodes[0] = 0;
	bmodes[1] = 0;
	bmodes[2] = 0;
	for (size_t	i = 0; i < modes.size(); i++)
		if (modes[i] == 'O')
			bmodes[0] = 1;
		else if (modes[i] == 'o')
			bmodes[1] = 1;
		else if (modes[i] == 'v')
			bmodes[2] = 1;
	_clients[client] = client_flags(bmodes);
}


Channel::~Channel() {}

Channel::Channel(Channel const &other)
{
	*this = other;
}

Channel::Channel(std::string const &name, std::string const &key, Client *client) :
	_name(name), _blocked(0), _creation_time(ft_getcurrenttime())
{
	_flags._key = key;
	if (name.size() > 0 && name[0] == '#')
		_clients[client] = client_flags(1, 1, 0);
	else if (name.size() > 0 && name[0] == '+')
		_clients[client];
	else if (name.size() > 0 && name[0] == '!')
	{
		_safe_postfix = get_safe_postfix();
		_clients[client] = client_flags(1, 1, 0);
	}
}

Channel	&Channel::operator=(Channel const &other)
{
	_name = other._name;
	_clients = other._clients;
	return (*this);
}

std::unordered_map<Client*, client_flags> &Channel::getclients(void)
{
	return _clients;
}

std::string	const &Channel::getname(void)
{
	return this->_name;
}

std::string	const &Channel::getkey(void)
{
	return _flags._key;
}

char		Channel::gettype(void)
{
	return _type;
}

void		Channel::settype(char type)
{
	_type = type;
}

void		Channel::add_client(Client *client)
{
	_clients[client];
}

bool		Channel::isSecret(void)
{
	return _flags._secret;
}

bool		Channel::isPrivate(void)
{
	return _flags._private;
}

std::string	const &Channel::gettopic(void)
{
	return _topic;
}

void		Channel::settopic(std::string const &topic)
{
	_topic = topic;
}

channel_flags const &Channel::getflags(void)
{
	return _flags;
}

Channel		*Channel::getptr(void)
{
	Channel	*ptr = (Channel*)this;
	return ptr;
}

void		Channel::block(void)
{
	_blocked = true;
}

void		Channel::unblock(void)
{
	_blocked = false;
}

bool		Channel::isBlocked(void)
{
	return this->_blocked;
}

bool		Channel::isOnChan(Client *client)
{
	std::unordered_map<Client*, client_flags>::iterator	it;

	for (it = _clients.begin(); it != _clients.end(); it++)
		if (it->first == client)
			return true;
	return false;
}

bool		Channel::isInvited(Client *client)
{
	return client->isInvited(this);
}

bool		Channel::isBanned(Client *client)
{
	return client->isBanned(this);
}
