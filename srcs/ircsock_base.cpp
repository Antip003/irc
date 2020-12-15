/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircsock_base.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salec <salec@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/14 00:09:46 by salec             #+#    #+#             */
/*   Updated: 2020/12/15 15:11:50 by salec            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.hpp"
#include "message.hpp"
#include "commands.hpp"
#include "tools.hpp"

void	ProcessMessage(int fd, std::string const &msg, IRCserv *serv)
{
	t_strvect	split = ft_splitstring(msg, " ");
	int			i = 0;

	/*	reply check here: reroute the reply to user or server	*/
	if ((serv->fds[fd].type = FD_ME || serv->fds[fd].type == FD_SERVER) &&
		split.size() > 3 && split[1].size() == 3 &&
		split[1].find_first_not_of("0123456789") == std::string::npos)
	{
		std::cout << "A reply " << split[1] << " from fd " << fd <<
			" recieved. Forward to " << split[2] << std::endl;
		Client	*found = find_client_by_nick(split[i], serv);
		if (found)
		{
			try
			{
				serv->fds.at(found->getFD()).wrbuf += strvect_to_string(split) + CRLF;
				//	getFD() of client from another server becomes the FD of that
				//	other server we connected to so we don't need to do more here
				//	just checking for existance with at()
			}
			catch (std::out_of_range &e) { (void)e; }
		}
		return ;	// for now because untested
	}


	if (split.size() > 0 && split[0][0] == ':')
		i = 1;
	split[i] = ft_strtoupper(split[i]);
	try
	{
		serv->command.at(split[i])(fd, split, serv);
		#if DEBUG_MODE
			std::cout << "command found:\t\t" << split[i] << std::endl;
		#endif
	}
	catch (std::out_of_range &e) { (void)e; }
}

void	CreateSock(IRCserv *serv, t_listen &_listen)
{
	t_sockaddr_in	sockin;
	t_protoent		*pe = NULL;
	int				optval = 1;

	if (!(pe = getprotobyname("tcp")))
		error_exit("getprotobyname error");
	if ((_listen.socket_fd = socket(PF_INET, SOCK_STREAM, pe->p_proto)) < 0)
		error_exit("socket error");
	if (fcntl(_listen.socket_fd, F_SETFL, O_NONBLOCK) < 0)
		error_exit("fcntl error: failed to set nonblock fd");
	sockin.sin_family = AF_INET;
	if (_listen.ip == "*")
		sockin.sin_addr.s_addr = INADDR_ANY;
	else
		sockin.sin_addr.s_addr = inet_addr(_listen.ip.c_str());
	sockin.sin_port = htons(_listen.port);
	if (setsockopt(_listen.socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
		error_exit("set socket option returned error");
	if (bind(_listen.socket_fd, (t_sockaddr*)&sockin, sizeof(sockin)) < 0)
		error_exit("bind error (probably already binded)");
	if (listen(_listen.socket_fd, 42) < 0) //42 can also be configured
		error_exit("listen error");
	serv->fds[_listen.socket_fd].type = FD_ME;
	serv->fds[_listen.socket_fd].tls = false;
	std::cout << "server created on socket " << _listen.socket_fd <<
		" (port " << _listen.port << ")" << std::endl;
}

void	AcceptConnect(int _socket, IRCserv *serv, bool isTLS)
{
	int				fd;
	t_sockaddr_in	csin;
	socklen_t		csin_len;

	csin_len = sizeof(csin);
	if (isTLS)
		fd = accept(_socket, (t_sockaddr*)&csin, &csin_len);
	else
		fd = accept(_socket, (t_sockaddr*)&csin, &csin_len);

	if (fd < 0)
		error_exit("accept error");
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		error_exit("fcntl error: failed to set nonblock fd");
	if (isTLS)
		std::cout << "tls";
	std::cout << "client " << fd << " accepted:\t" <<
		inet_ntoa(csin.sin_addr) << ":" << ntohs(csin.sin_port) << std::endl;
	serv->fds[fd].type = FD_CLIENT;
	// we dont know either it's client or other server
	serv->fds[fd].rdbuf.erase();
	serv->fds[fd].wrbuf.erase();
	serv->fds[fd].status = true;
	serv->fds[fd].tls = isTLS;
	serv->fds[fd].hostname = inet_ntoa(csin.sin_addr);
	serv->fds[fd].sslptr = NULL;

	if (isTLS)
	{
		if (!(serv->fds[fd].sslptr = SSL_new(serv->sslctx)))
		{
			ERR_print_errors_cb(SSLErrorCallback, NULL);
			error_exit("SSL_new failed");
		}
		if (SSL_set_fd(serv->fds[fd].sslptr, fd) < 1)
		{
			ERR_print_errors_cb(SSLErrorCallback, NULL);
			error_exit("SSL_set_fd failed");
		}
	}
}

void	self_cmd_squit(int fd, IRCserv *serv)
{
	t_strvect	split;
	for (size_t i = 0; i < serv->network.size(); i++)
	{
		if (serv->network[i].fd == fd)
		{
			split.push_back("SERVER");
			split.push_back(serv->network[i].servername);
			split.push_back(":Network split");
			cmd_squit(fd, split, serv);
			return ;
		}
	}
}

void	ReceiveMessage(int fd, IRCserv *serv)
{
	ssize_t		r = 0;
	char		buf_read[BUF_SIZE + 1];

	if (serv->fds[fd].tls && serv->fds[fd].sslptr)
		r = SSL_read(serv->fds[fd].sslptr, buf_read, BUF_SIZE);
	else
		r = recv(fd, buf_read, BUF_SIZE, 0);

	if (r >= 0)
		buf_read[r] = 0;
	if (r > 0)
	{
		serv->fds[fd].rdbuf += buf_read;
		if (serv->fds[fd].rdbuf.find_last_of(CRLF) != std::string::npos &&
			serv->fds[fd].rdbuf.find_last_of(CRLF) + 1 == serv->fds[fd].rdbuf.length())
		{
#if DEBUG_MODE
			if (serv->fds[fd].tls)
				std::cout << "tls";
			std::cout << "client " << fd << " sent:\t" <<
				(serv->fds[fd].tls ? "" : "\t") << serv->fds[fd].rdbuf;
#endif
			t_strvect	split = ft_splitstringbyany(serv->fds[fd].rdbuf, CRLF);
			for (size_t i = 0; i < split.size(); i++)
				ProcessMessage(fd, split[i], serv);
			//	ignore msgs with \r\n (maybe other symbols too)
			//	if (split[i].find_first_of(CRLF) == std::string::npos)
			//	not much use here
			try { serv->fds.at(fd).rdbuf.erase(); }
			catch (std::out_of_range const &e) { (void)e; }
		}
	}
	else
	{
		if (serv->fds[fd].tls)
		{
			/* for tls may be recoverable */
			int	err = SSL_get_error(serv->fds[fd].sslptr, r);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
				return ;
			ERR_print_errors_cb(SSLErrorCallback, NULL);
			SSL_free(serv->fds[fd].sslptr);
			std::cout << "tls";
		}
		//he we need to pop out introduced servers if type == FD_SERVER
		if (serv->fds[fd].type == FD_SERVER)
			self_cmd_squit(fd, serv);
		close(fd);
		serv->fds.erase(fd);
		t_citer	it = ft_findclientfd(serv->clients.begin(), serv->clients.end(), fd);
		if (it != serv->clients.end())
		{
			addtonickhistory(serv, it);
			serv->clients.erase(it);
		}
//		won't work for suddenly disconnecting servers	//	it->Disconnect();
		std::cout << "client " << fd << "\t\tdisconnected" << std::endl;
	}
}

void	SendMessage(int fd, IRCserv *serv)
{
	ssize_t		r = 0;
	std::string	reply;

	if (serv->fds[fd].wrbuf.length() > BUF_SIZE)
	{
		reply = serv->fds[fd].wrbuf.substr(0, BUF_SIZE);
		serv->fds[fd].wrbuf = serv->fds[fd].wrbuf.substr(BUF_SIZE);
	}
	else
	{
		reply = serv->fds[fd].wrbuf;
		serv->fds[fd].wrbuf.erase();
	}
#if DEBUG_MODE
	if (serv->fds[fd].tls)
		std::cout << "tls";
	std::cout << "sending client " << fd << "\t" << reply;
#endif

	if (serv->fds[fd].tls && serv->fds[fd].sslptr)
		r = SSL_write(serv->fds[fd].sslptr, reply.c_str(), reply.length());
	else
		r = send(fd, reply.c_str(), reply.length(), 0);

	if (r <= 0 || serv->fds[fd].status == false)
	{
		if (serv->fds[fd].tls)
		{
			/* for tls may be recoverable */
			int	err = SSL_get_error(serv->fds[fd].sslptr, r);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
				return ;
			ERR_print_errors_cb(SSLErrorCallback, NULL);
			SSL_free(serv->fds[fd].sslptr);
			std::cout << "tls";
		}
		if (serv->fds[fd].type == FD_SERVER)
			self_cmd_squit(fd, serv);
		close(fd);
		serv->fds.erase(fd);
		t_citer	it = ft_findclientfd(serv->clients.begin(), serv->clients.end(), fd);
		if (it != serv->clients.end())
		{
			addtonickhistory(serv, it);
			serv->clients.erase(it);
		}
//		won't work for suddenly disconnecting servers	//	it->Disconnect();
		std::cout << "client " << fd << ":\t\tdisconnected" << std::endl;
	}
}

bool	didSockFail(int fd, IRCserv *serv)
{
	if (serv->fds[fd].status == false && serv->fds[fd].type == FD_SERVER)
	{
		int			error;
		socklen_t	len = sizeof(error);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			error_exit("getsockopt failure");
		if (error)
		{
			std::cerr << "Connection error to server " << fd << std::endl;
			msg_error("Connection error to server", serv);
			close(fd);
			if (serv->fds[fd].tls)
			{
				SSL_free(serv->fds[fd].sslptr);
				serv->fds.erase(fd);
			}
			return (true);
		}
		else
			serv->fds[fd].status = true;
	}
	return (false);
}
