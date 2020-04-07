/* Guerre de Tank, bataille de tank en réseau. Copyright (C) 2018 Rémi Pérenne. 
Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le modifier au titre des clauses de la Licence Publique Générale GNU, telle que publiée par la Free Software Foundation ; soit la version 2 de la Licence, ou (à votre discrétion) une version ultérieure quelconque. Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE GARANTIE ; sans même une garantie implicite de COMMERCIABILITE ou DE CONFORMITE A UNE UTILISATION PARTICULIERE. Voir la Licence Publique Générale GNU pour plus de détails. Vous devriez avoir reçu un exemplaire de la Licence Publique Générale GNU avec ce programme ; si ce n'est pas le cas, écrivez à la Free Software Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include <SFML/Network.hpp>
#include "irrlicht/irrlicht.h"

#ifndef CLIENT
#define CLIENT

enum EQUIPE{BLEU, ROUGE};

typedef struct Client Client;
struct Client
{
    sf::IpAddress adresse;
    unsigned short port;
    sf::String pseudo;
    bool equipe;
    sf::Int32 id;
    sf::Int32 positionDepart;
};

sf::Packet& operator<<(sf::Packet& packet, const Client& a);

sf::Packet& operator>>(sf::Packet& packet, Client& a);

#endif // CLIENT
