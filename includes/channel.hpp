/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salec <salec@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/24 12:40:08 by salec             #+#    #+#             */
/*   Updated: 2020/12/01 23:04:28 by gbright          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <vector>
#include "client.hpp"

/*	A channel is a named group of one or more clients which will all
	receive messages addressed to that channel.  The channel is created
	implicitly when the first client joins it, and the channel ceases to
	exist when the last client leaves it.  While channel exists, any
	client can reference the channel using the name of the channel.
	Channels names are strings (beginning with a '&' or '#' character) of
	length up to 200 characters.  Apart from the the requirement that the
	first character being either '&' or '#'; the only restriction on a
	channel name is that it may not contain any spaces (' '), a control G
	(^G or ASCII 7), or a comma (',' which is used as a list item
	separator by the protocol).	*/

class Channel {
private:
	std::string				_name;
	std::string				_key;
	char					_type;
	Client					&_chop;		// channel operator
	std::vector<Client*>	_clients;
	Channel();
public:
	Channel(std::string const &name, Client &creator);
	~Channel();
	Channel(Channel const &other);
	Channel &operator=(Channel const &other);
	
	std::string const	&getname(void);
	std::string	const	&getkey(void);
	char				gettype(void);
	void				add_client(Client *client);

	void	settype(char type);
};

#endif
