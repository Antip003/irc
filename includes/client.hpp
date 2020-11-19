/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salec <salec@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 12:10:42 by salec             #+#    #+#             */
/*   Updated: 2020/11/19 09:32:41 by gbright          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
// fcntl, select, socket, inet structs, inet
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
// read, write
#include <unistd.h>

#include "common_defines.hpp"

/*	A client is anything connecting to a server that is not another
	server.  Each client is distinguished from other clients by a unique
	nickname having a maximum length of nine (9) characters.
	See the protocol grammar rules for what may and may not be used in a
	nickname.  In addition to the nickname, all servers must have the
	following information about all clients: the real name of the host
	that the client is running on, the username of the client on that
	host, and the server to which the client is connected.	*/

class Client {
private:
	std::string		nickname;
	std::string		username;
	std::string		realname;
	std::string		hostname;
	int				fd;
	bool			_isConnected;
	bool			_isRegistred;
	bool			_isOperator;
	bool			USER;
	bool			NICK;
	// Maybe server class object reference or pointer here
	Client(void);
public:
	Client(std::string const &nickname, int fd);
	Client(std::string const &username, std::string const &realname, int fd);
	Client(int fd, std::string const &username);
	~Client();
	Client(Client const &other);
	Client &operator=(Client const &other);

	// getters-setters here
	bool				isConnected(void);
	bool				isRegistred(void);
	bool				isOperator(void);
	bool				getUSER(void);
	bool				getNICK(void);
	int const	 		&getFD(void);
	void				setFD(int fd);
	void				setOPER(void);
	std::string const	&getnickname(void);
	std::string const	&getusername(void);
	std::string const	&getrealname(void);

	bool				Register(std::string const &user,
							std::string const &real);
	bool				Register(std::string const &nick);
	void				Reconnect(int fd);
	void				Disconnect(void);
	void				ChangeNick(std::string const &what);
};

#endif
