#include "ircserv.hpp"
#include "commands.hpp"
#include "message.hpp"
#include "tools.hpp"

void    squit_from_client(int fd, const t_strvect &split, IRCserv *serv);

void	squit_from_network(int fd, const t_strvect &split, IRCserv *serv)
{
	t_strvect	command;
	std::vector<t_server>::iterator	net;

	if (split.size() < 4)
		return ;
	for (net = serv->network.begin(); net != serv->network.end(); ++net)
		if (net->servername == split[2])
		{
			command.push_back("SQUIT");
			command.push_back(split[2]);
			command.push_back(split[3]);
			squit_from_client(serv->listen[0].socket_fd, command, serv);
			return ;
		}
	msg_forward(fd, strvect_to_string(split), serv);
}

void	squit_from_client(int fd, const t_strvect &split, IRCserv *serv)
{
	Client	*client = 0;
	t_server	*_serv;
	std::string	squit_msg;
	std::list<Client>::iterator client_it;

	if (serv->fds[fd].type != FD_OPER && serv->fds[fd].type != FD_ME)
	{
		serv->fds[fd].wrbuf += get_reply(serv, ERR_NOPRIVILEGES, fd, "",
				"Permission Denied- You're not an IRC operator"); return ;
	}
	if (split.size() < 2 && serv->fds[fd].type == FD_OPER)
	{
		serv->fds[fd].wrbuf += get_reply(serv, ERR_NEEDMOREPARAMS, fd, "SQUIT",
			"Not enough parameters"); return ;
	}
	if (split.size() == 2)
		squit_msg = ":Network split";
	else if (split.size() > 2)
		squit_msg = split[2];
	if (!(_serv = find_server_by_name(split[1], serv)))
	{
		serv->fds[fd].wrbuf+= get_reply(serv, ERR_NOSUCHSERVER, fd, split[1],
			"No such server"); return ;
	}
	else if (_serv->servername != split[1] && serv->fds[fd].type == FD_OPER)
	{
		serv->fds[_serv->fd].wrbuf += ":" + client->getnick() +
			strvect_to_string(split) + CRLF; return ;
	}
	else if (_serv->servername == split[1] && serv->fds[fd].type == FD_ME)
	{
		std::list<Channel>::iterator	chan;
		std::list<blocked>::iterator	blocked_it;
		blocked						temp;

		temp.servername = split[1];
		client_it = _serv->clients.begin();
		for (; client_it != _serv->clients.end(); client_it++)
		{
			client_it->block();
			temp.clients.push_back(&(*client_it));
		}
		chan = serv->channels.begin();
		for (; chan != serv->channels.end(); chan++)
			if (chan->block_if())
				temp.channels.push_back(&(*chan));
		temp.fd = _serv->fd;
		temp._blocked_time = ft_getcurrenttime();
		serv->unavailable.push_back(temp);
		serv->fds[_serv->fd].blocked = true;
		serv->fds[_serv->fd].fatal = false;
	}
	else if (_serv->servername == split[1] && serv->fds[fd].type == FD_OPER)
	{
		t_strvect	command;

		if (!(client = find_client_by_fd(fd, serv)))
			return ;
		client_it = _serv->clients.begin();
		for (; client_it != _serv->clients.end(); client_it++)
		{
			command.push_back(":" + client_it->getnick());
			command.push_back("QUIT");
			command.push_back(":Server " + _serv->servername + " is dead");
			cmd_quit(_serv->fd, split, serv);
			command.clear();
		}
		remove_server_by_name(_serv->servername, serv);
		serv->fds[_serv->fd].wrbuf += ":" + client->getnick() + " " +
			strvect_to_string(split) + CRLF;
		msg_forward(-1, ":" + client->getnick() + " " + strvect_to_string(split), serv);
		serv->fds[_serv->fd].blocked = false;
		serv->fds[_serv->fd].status = false;
		serv->fds[_serv->fd].fatal = false;
	}
}

void	cmd_squit(int fd, const t_strvect &split, IRCserv *serv)
{
	if (serv->fds[fd].type == FD_UNREGISTRED)
	{
		serv->fds[fd].wrbuf += get_reply(serv, ERR_NOTREGISTERED, -1, "",
				"You have not registered"); return ;
	}
	if (serv->fds[fd].type == FD_SERVER && split[0][0] == ':')
		squit_from_network(fd, split, serv);
	else
		squit_from_client(fd, split, serv);
}
