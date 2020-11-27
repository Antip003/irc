/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_userhost.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salec <salec@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/26 15:47:09 by salec             #+#    #+#             */
/*   Updated: 2020/11/26 19:35:48 by salec            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.hpp"
#include "commands.hpp"
#include "message.hpp"

/*	Command: USERHOST
	Parameters: <nickname> *( SPACE <nickname> )

	The USERHOST command takes a list of up to 5 nicknames, each
	separated by a space character and returns a list of information
	about each nickname that it found.  The returned list has each reply
	separated by a space.

	Numeric Replies:
		RPL_USERHOST
		ERR_NEEDMOREPARAMS

	Example:
	USERHOST Wiz Michael syrk
	; USERHOST request for information on nicks "Wiz", "Michael", and "syrk"

	:ircd.stealth.net 302 yournick :syrk=+syrk@millennium.stealth.net
	; Reply for user syrk
*/

typedef std::vector<Client>::iterator	t_cvit;

void	cmd_userhost(int fd, const t_strvect &split, IRCserv *serv)
{
	t_citer	cit = ft_findclientfd(serv->clients.begin(), serv->clients.end(), fd);
	if (cit == serv->clients.end())
		return ;

	if (split.size() < 2)
	{
		serv->fds[fd].wrbuf += ft_buildmsg(serv->servername,
			ERR_NEEDMOREPARAMS, cit->getnickname(), "USERHOST",
			"Not enough parameters");
		return ;
	}

	t_strvect::const_iterator	argit = split.begin() + 1;
	for (int i = 1; i < 6 && argit != split.end(); i++)
	{
		for (t_cvit it = serv->clients.begin(); it != serv->clients.end(); it++)
		{
			if (it->getnickname() == split[i])
			{
				serv->fds[fd].wrbuf += ft_buildmsg(serv->servername,
					RPL_USERHOST, cit->getnickname(), "",
					it->getnickname() + "=+" +
					it->getusername() + "@" + it->gethostname());
			}
		}
		argit++;
	}
}