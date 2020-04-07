/* Guerre de Tank, bataille de tank en réseau. Copyright (C) 2018 Rémi Pérenne. 
Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le modifier au titre des clauses de la Licence Publique Générale GNU, telle que publiée par la Free Software Foundation ; soit la version 2 de la Licence, ou (à votre discrétion) une version ultérieure quelconque. Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE GARANTIE ; sans même une garantie implicite de COMMERCIABILITE ou DE CONFORMITE A UNE UTILISATION PARTICULIERE. Voir la Licence Publique Générale GNU pour plus de détails. Vous devriez avoir reçu un exemplaire de la Licence Publique Générale GNU avec ce programme ; si ce n'est pas le cas, écrivez à la Free Software Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "infoPartie.h"
#include "vector3d_operateur.h"

sf::Packet& operator<<(sf::Packet& packet, const infoPartie& a)
{
    sf::Int32 nbClients = a.listeCLients.size();
    packet<<nbClients;
    for(int i = 0; i < nbClients; ++i)
    {
        packet<<a.listeCLients[i];
    }

    for(int i = 0; i < 10; i++)
    {
        packet<<a.listePositionPaqueMunition[i];
    }

    packet<<a.limiteRafraichissement;
    packet<<a.nbEliminationGagnePartie;

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, infoPartie& a)
{
    sf::Int32 nbClient;
    packet>>nbClient;
    for(int i = 0; i < nbClient; ++i)
    {
        Client client;
        packet>>client;
        a.listeCLients.push_back(client);
    }

    for(int i = 0; i < 10; ++i)
    {
        irr::core::vector3df pos;
        packet>>pos;
        a.listePositionPaqueMunition.push_back(pos);
    }

    packet>>a.limiteRafraichissement;
    packet>>a.nbEliminationGagnePartie;

    return packet;
}
