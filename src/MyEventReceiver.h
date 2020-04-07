/* Guerre de Tank, bataille de tank en réseau. Copyright (C) 2018 Rémi Pérenne. 
Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le modifier au titre des clauses de la Licence Publique Générale GNU, telle que publiée par la Free Software Foundation ; soit la version 2 de la Licence, ou (à votre discrétion) une version ultérieure quelconque. Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE GARANTIE ; sans même une garantie implicite de COMMERCIABILITE ou DE CONFORMITE A UNE UTILISATION PARTICULIERE. Voir la Licence Publique Générale GNU pour plus de détails. Vous devriez avoir reçu un exemplaire de la Licence Publique Générale GNU avec ce programme ; si ce n'est pas le cas, écrivez à la Free Software Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef RECEIVER
#define RECEIVER

#include <SFML/Network.hpp>
#include <irrlicht/irrlicht.h>
#include <vector>
#include "client.h"

class MyEventReceiver : public irr::IEventReceiver
{
public:
    MyEventReceiver(irr::gui::IGUIEnvironment* gui, irr::ITimer* timer, irr::video::IVideoDriver* driver);
    ~MyEventReceiver();
    virtual bool OnEvent(const irr::SEvent& event);
    bool miseAJour(irr::core::dimension2d<irr::u32> screenSize);
    void lancerServeur();
    void attendreConnection();
    void choixEquipe();
    void arreterServeur();
    void verifierSiClientDeconnecte();
    void creerEquipe();

    unsigned short getPort();
    std::vector<Client> getListeClients();

private:
    //partie importante irrlicht
    irr::gui::IGUIEnvironment* m_gui;
    irr::video::IVideoDriver* m_driver;
    irr::ITimer* m_timer;

    //reseau
    sf::TcpListener m_listener;
    sf::TcpSocket m_listeClientsTcp[40];

    //partie 1
    irr::gui::IGUIButton* m_bLancerServeur;
    irr::gui::IGUIEditBox* m_ebPort;
    bool m_ebPortFocused;
    unsigned short m_port;
    irr::gui::IGUISpinBox* m_sbNbJoueursAttendu, *m_sbNbEliminationGagnePartie;
    sf::Uint32 m_nbJoueursAttendu;
    irr::gui::IGUIStaticText* m_textInfoNbJoueurs;
    sf::Int32 m_nbEliminationPourGagnePartie;

    //partie 2
    std::vector<Client> m_listeCLients;
    irr::s8 m_indiceAnimAttendreConnection;
    irr::gui::IGUIStaticText *m_textAttendreConnection;
    irr::gui::IGUIListBox* m_lbClientsConnecte;
    sf::Uint32 m_nbCLientsConnecte;
    irr::gui::IGUIButton *m_bAnnuler;
    irr::u32 m_heureDerniereRafraichissementAnimAttendreConnection;

    //partie 3
    std::vector<sf::Int32> m_listeCLientsEquipeBleu;
    irr::gui::IGUIListBox *m_lbEquipeBleu, *m_lbEquipeRouge, *m_lbFocused;
    irr::gui::IGUIButton *m_bDemarerPartie;

    //autre
    std::vector<int> m_listeClientDeconnecte;
    bool m_etape1, m_etape2, m_etape3, m_lancerPartie;
    irr::gui::IGUIStaticText *m_textVersion, *m_textPresentation, *m_textInfo;
    irr::u32 m_heureDernierAppuiTouchePrisEnCompte;
    irr::core::stringw m_textPresentation1, m_textPresentation2, m_textPresentation3, m_animAttendreConnection[4];

};

#endif // RECEIVER
