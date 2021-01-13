/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: salec <salec@student.21-school.ru>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/09 18:57:17 by salec             #+#    #+#             */
/*   Updated: 2021/01/13 17:25:56 by salec            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "tools.hpp"
#include <stdlib.h>
#include <sstream>

void		error_exit(std::string const &s1)
{
	std::cerr << s1 << std::endl;
	exit(1);
}

int			ft_stoi(std::string const &str)
{
	std::istringstream	iss(str);
	int					val = 0;

	if (!(iss >> val))
		return (0);
	return (val);
}

t_strvect	ft_splitcmdbyspace(std::string msg)
{
	t_strvect		split;
	size_t			pos = 0;
	std::string		tmp;

	while ((pos = msg.find_first_of(" ")) != std::string::npos)
	{
		tmp = msg.substr(0, pos);
		if (!tmp.empty())
			split.push_back(tmp);
		msg.erase(0, pos + 1);
		if (!msg.empty() && msg[0] == ':')
			break ;
	}
	if (!msg.empty())
		split.push_back(msg);
	if (split.back().size() > 0 && split.back()[0] == ':')
		split.back() = split.back().substr(1);
	return (split);
}