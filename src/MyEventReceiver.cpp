/* Guerre de Tank, bataille de tank en réseau. Copyright (C) 2018 Rémi Pérenne.
Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le modifier au titre des clauses de la Licence Publique Générale GNU, telle que publiée par la Free Software Foundation ; soit la version 2 de la Licence, ou (à votre discrétion) une version ultérieure quelconque. Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE GARANTIE ; sans même une garantie implicite de COMMERCIABILITE ou DE CONFORMITE A UNE UTILISATION PARTICULIERE. Voir la Licence Publique Générale GNU pour plus de détails. Vous devriez avoir reçu un exemplaire de la Licence Publique Générale GNU avec ce programme ; si ce n'est pas le cas, écrivez à la Free Software Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "MyEventReceiver.h"
#include <SFML/Network.hpp>
#include <string>
#include "vector3d_operateur.h"
#include "infoPartie.h"


using namespace irr;

MyEventReceiver::MyEventReceiver(irr::gui::IGUIEnvironment* gui, irr::ITimer* timer, irr::video::IVideoDriver* driver) : IEventReceiver()
{
    //partie importante irrlicht
    m_gui = gui;
    m_timer = timer;
    m_driver = driver;

    //reseau
    m_listener.setBlocking(false);

    //variable qui vont servirent tous a chaque etape
    m_lancerPartie = false;
    sf::String ip_str = sf::IpAddress::getPublicAddress().toString();
    std::wstring ip_wstr, text;
    ip_wstr = ip_str.toWideString();
    text = L"Etape 1/3 : Configuration du serveur\n\n  -Port :\n\n  -Nombre de joueurs :\n\n  -Nombre d'elimination (par équipe) pour gagner la partie :\n\nNote : Votre adresse IP public est : ";
    text += ip_wstr;
    ip_str = sf::IpAddress::getLocalAddress().toString();
    ip_wstr = ip_str.toWideString();
    text += L", votre adresse IP local est :";
    text += ip_wstr;
    m_textPresentation1 = text.c_str();
    m_textPresentation2 = L"Etape 2/3 : Attente des clients";
    m_textPresentation3 = L"Etape 3/3 : Création des équipes\n\n                                                             joueurs :\n\n   joueurs équipe Rouge :                                                                                            joueurs équipe Bleu :";

    m_textVersion = gui->addStaticText(L"-Version Serveur 1.0", core::rect<s32>(0,0,150,20));

    m_etape1 = true;
    m_etape2 = false;
    m_etape3 = false;
    m_textPresentation = gui->addStaticText(m_textPresentation1.c_str(), core::rect<s32>(0,0,600,130), false);

    //

    //partie 1
    core::stringw text_wstr = L"La partie sera rafraichie ";
    text_wstr += m_driver->getFPS() / 2;
    text_wstr += L" fois par seconde";

    m_textInfoNbJoueurs = gui->addStaticText(text_wstr.c_str(), core::rect<s32>(0,0,390,300));
    m_textInfo = gui->addStaticText(L"", core::rect<s32>(0,0,390,300));
    m_ebPort = gui->addEditBox(L"50000", core::rect<s32>(0,0,100,20), true, 0, 1);
    m_sbNbJoueursAttendu = gui->addSpinBox(L"2", core::rect<s32>(0,0,50,20), true, 0, 510);
    m_sbNbJoueursAttendu->setDecimalPlaces(0);
    m_sbNbJoueursAttendu->setRange(1, 40);
    m_sbNbEliminationGagnePartie = gui->addSpinBox(L"2", core::rect<s32>(0,0,50,20), true, 0, 510);
    m_sbNbEliminationGagnePartie->setDecimalPlaces(0);
    m_sbNbEliminationGagnePartie->setRange(1, 99);
    m_sbNbEliminationGagnePartie->setValue(5);
    m_ebPortFocused = false;
    m_bLancerServeur = gui->addButton(core::rect<s32>(0,0,500,50), 0, 10, L"Lancer le serveur", L"Cliquer pour lancer le serveur\net commencer à attendre la connexion des clients");

    //partie 2
    m_bAnnuler = gui->addButton(core::rect<s32>(0,0,150,40), 0, 11, L"Annuler", L"Cliquer pour fermer le serveur");
    m_bAnnuler->setVisible(false);
    m_lbClientsConnecte = gui->addListBox(core::rect<s32>(0,0,180,230), 0, 20, false);
    m_lbClientsConnecte->setToolTipText(L"Selectionner le pseudo d'un joueur\net utilisez les flèches directionnelles\npour lui affecter l'une ou l'autre des équipes");
    m_lbClientsConnecte->setVisible(false);
    m_nbCLientsConnecte = 0;
    m_animAttendreConnection[0] = L"";
    m_animAttendreConnection[1] = L".";
    m_animAttendreConnection[2] = L"..";
    m_animAttendreConnection[3] = L"...";
    m_indiceAnimAttendreConnection = 0;
    m_textAttendreConnection = gui->addStaticText(0, core::rect<s32>(0,0,150,20));
    m_textAttendreConnection->setVisible(false);
    m_heureDerniereRafraichissementAnimAttendreConnection = 0;

    //partie 3
    m_bDemarerPartie = gui->addButton(core::rect<s32>(0,0,400, 50), 0, 50, L"Démarer la partie", L"Si tous les joueurs on une équipe\net qu'il y a au moin 1 joueur dans chaque équipe\nvous pouvez cliquer pour lancer la partie");
    m_bDemarerPartie->setEnabled(false);
    m_bDemarerPartie->setVisible(false);
    m_lbEquipeBleu = gui->addListBox(core::rect<s32>(0,0,150, 200), 0, 21);
    m_lbEquipeBleu->setVisible(false);
    m_lbEquipeRouge = gui->addListBox(core::rect<s32>(0,0,150, 200), 0, 22);
    m_lbEquipeRouge->setVisible(false);
    m_lbFocused = m_lbClientsConnecte;
}

bool MyEventReceiver::miseAJour(core::dimension2du screenSize)
{
    //RESEAU
    if(m_etape2)
    {
        attendreConnection();
    }

    //MISE A JOUR GUI
    s32 screenSize_middleWidth = screenSize.Width/2;
    s32 screenSize_middleHeight = screenSize.Height/2;

    core::position2di positionElement;

    if(m_etape1)
    {
        positionElement.X = screenSize_middleWidth - 120;
        positionElement.Y = screenSize_middleHeight - 100;
        m_textInfoNbJoueurs->setRelativePosition(positionElement);

        positionElement.Y = screenSize_middleHeight + 80;
        positionElement.X = screenSize_middleWidth - 250;
        m_textInfo->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 255;
        positionElement.Y = screenSize_middleHeight - 135;
        m_ebPort->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 185;
        positionElement.Y = screenSize_middleHeight - 105;
        m_sbNbJoueursAttendu->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth + 5;
        positionElement.Y = screenSize_middleHeight - 78;
        m_sbNbEliminationGagnePartie->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 300;
        positionElement.Y = screenSize_middleHeight;
        m_bLancerServeur->setRelativePosition(positionElement);
    }

    if(m_etape2)
    {
        verifierSiClientDeconnecte();

        positionElement.X = screenSize_middleWidth - 20;
        positionElement.Y = screenSize_middleHeight - 50;
        m_textInfo->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 280;
        positionElement.Y = screenSize_middleHeight - 110;
        m_lbClientsConnecte->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 80;
        positionElement.Y = screenSize_middleHeight + 180;
        m_bAnnuler->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 280;
        positionElement.Y = screenSize_middleHeight - 130;
        m_textAttendreConnection->setRelativePosition(positionElement);

        if(m_timer->getTime() > m_heureDerniereRafraichissementAnimAttendreConnection + 300)
        {
            m_heureDerniereRafraichissementAnimAttendreConnection = m_timer->getTime();

            core::stringw text = L"(";
            text += m_nbCLientsConnecte;
            text += L"/";
            text += m_nbJoueursAttendu;
            text += L") joueurs connectés";
            text += m_animAttendreConnection[m_indiceAnimAttendreConnection];

            m_textAttendreConnection->setText(text.c_str());

            m_indiceAnimAttendreConnection++;
            if(m_indiceAnimAttendreConnection > 3) m_indiceAnimAttendreConnection = 0;
        }
    }

    if(m_etape3)
    {
        verifierSiClientDeconnecte();

        positionElement.X = screenSize_middleWidth + 250;
        positionElement.Y = screenSize_middleHeight - 100;
        m_textInfo->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 110;
        positionElement.Y = screenSize_middleHeight + 200;
        m_bAnnuler->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 130;
        positionElement.Y = screenSize_middleHeight - 110;
        m_lbClientsConnecte->setRelativePosition(positionElement);

        positionElement.Y = screenSize_middleHeight - 85;
        positionElement.X = screenSize_middleWidth - 310;
        m_driver->draw2DRectangle(video::SColor(50,255,0,0), core::rect<s32>(positionElement.X, positionElement.Y, positionElement.X + 150, positionElement.Y + 200));
        m_lbEquipeRouge->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth + 80;
        m_driver->draw2DRectangle(video::SColor(50,0,0,255), core::rect<s32>(positionElement.X, positionElement.Y, positionElement.X + 150, positionElement.Y + 200));
        m_lbEquipeBleu->setRelativePosition(positionElement);

        positionElement.X = screenSize_middleWidth - 230;
        positionElement.Y = screenSize_middleHeight + 135;
        m_bDemarerPartie->setRelativePosition(positionElement);
    }

    positionElement.X = screenSize_middleWidth - 300;
    positionElement.Y = screenSize_middleHeight - 160;
    m_textPresentation->setRelativePosition(positionElement);

    positionElement.X = screenSize_middleWidth + 210;
    positionElement.Y = screenSize_middleHeight - 190;
    m_textVersion->setRelativePosition(positionElement);

    if(m_lancerPartie)
    {
        creerEquipe();

        sf::Packet packet;
        infoPartie info;

        //
        info.listeCLients = m_listeCLients;

        //
        //on tire au hasard la position des paque de munition pour tous les clients
        std::vector<core::vector3df> listePosPaque;
        for(int i = 0; i<10; ++i)
        {
            core::vector3df position;
            //on tire les position x et z au pif
            position.X = (rand() % (11500 - 500 + 1)) + 500;
            position.Z = (rand() % (11500 - 500 + 1)) + 500;

            listePosPaque.push_back(position);
        }

        info.listePositionPaqueMunition = listePosPaque;

        //
        sf::Int32 FPS = m_driver->getFPS();
        sf::Int32 intervalleRafraichissementClient = FPS/m_nbCLientsConnecte;

        info.limiteRafraichissement = intervalleRafraichissementClient;
        info.nbEliminationGagnePartie = m_nbEliminationPourGagnePartie;

        packet<<info;
        packet>>info;
        for(unsigned int i = 0; i < m_nbCLientsConnecte; ++i)
        {
            m_listeClientsTcp[i].send(packet);
        }

        irr::core::list<irr::gui::IGUIElement*> listeElements = m_gui->getRootGUIElement()->getChildren();
        for(core::list<gui::IGUIElement*>::Iterator it = listeElements.begin(); it != listeElements.end(); ++it)
        {
            (*it)->remove();
        }

        return true;
    }

    return false;
}

bool MyEventReceiver::OnEvent(const irr::SEvent& event)
{
    if(event.EventType == EET_GUI_EVENT)
    {
        s32 id = event.GUIEvent.Caller->getID();
        switch(event.GUIEvent.EventType)
        {
        case gui::EGET_BUTTON_CLICKED:
            switch(id)
            {
            case 10:
                lancerServeur();
                break;
            case 11:
                arreterServeur();
                break;
            case 50:
                m_lancerPartie = true;
                break;
            }
            return true;

        case gui::EGET_LISTBOX_CHANGED:
            switch(id)
            {
            case 20:
                m_lbFocused = m_lbClientsConnecte;
                m_lbEquipeBleu->setSelected(-1);
                m_lbEquipeRouge->setSelected(-1);
                break;
            case 21:
                m_lbFocused = m_lbEquipeBleu;
                m_lbEquipeRouge->setSelected(-1);
                m_lbClientsConnecte->setSelected(-1);
                break;
            case 22:
                m_lbFocused = m_lbEquipeRouge;
                m_lbEquipeBleu->setSelected(-1);
                m_lbClientsConnecte->setSelected(-1);
                break;
            }
            return true;

        case gui::EGET_EDITBOX_ENTER :
            if(id == 1)
            {
                lancerServeur();
            }
            return true;

        case gui::EGET_SPINBOX_CHANGED :
            if(id == 510)
            {
                irr::core::stringw text = L"La partie sera rafraichie ";
                text += m_driver->getFPS() / s32(m_sbNbJoueursAttendu->getValue());
                text += " fois par seconde";
                m_textInfoNbJoueurs->setText(text.c_str());
            }

        default : return false;
        }
    }

    if(event.EventType == EET_KEY_INPUT_EVENT)
    {
        irr::EKEY_CODE key = event.KeyInput.Key;

        if(m_etape3)
        {
            if(key == irr::KEY_RIGHT || key == irr::KEY_UP)
            {
                irr::s32 itemSelecionne = m_lbFocused->getSelected();
                if(itemSelecionne != -1)
                {
                    const wchar_t* item = m_lbFocused->getListItem(itemSelecionne);
                    m_lbEquipeBleu->addItem(item);
                    m_lbFocused->removeItem(itemSelecionne);
                }
            }

            if(key == irr::KEY_LEFT || key == irr::KEY_DOWN)
            {
                irr::s32 itemSelecionne = m_lbFocused->getSelected();
                if(itemSelecionne != -1)
                {
                    const wchar_t* item = m_lbFocused->getListItem(itemSelecionne);
                    m_lbEquipeRouge->addItem(item);
                    m_lbFocused->removeItem(itemSelecionne);
                }
            }

            if(m_lbClientsConnecte->getItemCount() == 0 && m_lbEquipeBleu->getItemCount() > 0 && m_lbEquipeRouge->getItemCount() > 0)
            {
                m_bDemarerPartie->setEnabled(true);
            }

            else
            {
                m_bDemarerPartie->setEnabled(false);
            }
            return true;
        }
    }
    return false;
}

void MyEventReceiver::lancerServeur()
{
    std::wstring text_wstring = m_ebPort->getText();
    unsigned short port = wcstol(text_wstring.c_str(), 0, 10);
    m_port = port;

    if(m_listener.listen(port) != sf::Socket::Done)
    {
        std::wstring text = L"Le port ";
        text += text_wstring;
        text += L" est indisponible, essayez un autre port";
        m_textInfo->setText(text.c_str());
    }

    else
    {
        sf::String ip_str;
        std::wstring wstr = L"Le serveur écoute les connections entrantes\nsur le port : ";
        wstr += text_wstring;
        wstr += L" aux adresses : ";
        ip_str = sf::IpAddress::getLocalAddress().toString();
        wstr += ip_str.toWideString();
        wstr += L"\n(local) et ";
        ip_str = sf::IpAddress::getPublicAddress().toString();
        wstr += ip_str.toWideString();
        wstr += L" (public)";
        m_textInfo->setText(wstr.c_str());

        text_wstring = m_sbNbJoueursAttendu->getText();
        m_nbJoueursAttendu = wcstol(text_wstring.c_str(), 0, 10);

        text_wstring = m_sbNbEliminationGagnePartie->getText();
        m_nbEliminationPourGagnePartie = wcstol(text_wstring.c_str(), 0, 10);

        m_ebPort->setVisible(false);
        m_sbNbJoueursAttendu->setVisible(false);
        m_sbNbEliminationGagnePartie->setVisible(false);
        m_bLancerServeur->setVisible(false);
        m_textInfoNbJoueurs->setVisible(false);

        m_textPresentation->setText(m_textPresentation2.c_str());
        m_lbClientsConnecte->setVisible(true);
        m_textAttendreConnection->setVisible(true);
        m_bAnnuler->setVisible(true);

        m_etape1 = false;
        m_etape2 = true;
    }
}

void MyEventReceiver::attendreConnection()
{
    sf::Packet packet;

    if(m_listener.accept(m_listeClientsTcp[m_nbCLientsConnecte]) == sf::Socket::Done)
    {
        if(m_listeClientsTcp[m_nbCLientsConnecte].receive(packet) == sf::Socket::Done)
        {
            sf::String pseudo;
            packet>>pseudo;
            irr::s32 n = 1;
            for(std::vector<Client>::iterator it = m_listeCLients.begin(); it != m_listeCLients.end(); ++it)
            {
                if(pseudo.find((*it).pseudo) != std::string::npos)
                {
                    ++n;
                }
            }
            if(n > 1)
            {
                irr::core::stringw strw;
                strw += n;
                std::wstring wstr(strw.c_str());
                pseudo += wstr;
            }

            std::wstring pseudo_wstr = pseudo.toWideString();
            m_lbClientsConnecte->addItem(pseudo_wstr.c_str());

            Client client;
            client.adresse = m_listeClientsTcp[m_nbCLientsConnecte].getRemoteAddress();
            client.port = m_listeClientsTcp[m_nbCLientsConnecte].getRemotePort();
            client.pseudo = pseudo;
            client.id = m_nbCLientsConnecte;
            client.equipe = false;
            m_listeCLients.push_back(client);
        }

        //on donne un id au client qui vient de se connecter
        packet.clear();
        packet<<m_nbCLientsConnecte;
        m_listeClientsTcp[m_nbCLientsConnecte].send(packet);

        m_listeClientsTcp[m_nbCLientsConnecte].setBlocking(false);
        m_nbCLientsConnecte++;

        //on dit a chaque joueurs q un nouveau client s est connecte
        for(unsigned int i = 0; i < m_nbCLientsConnecte; i++)
        {
            sf::Packet packet;
            packet<<m_nbCLientsConnecte;
            packet<<m_nbJoueursAttendu;
            m_listeClientsTcp[i].send(packet);
        }
    }

    if(m_nbCLientsConnecte == m_nbJoueursAttendu)
    {
        choixEquipe();
    }
}

void MyEventReceiver::choixEquipe()
{
    m_listener.close();

    m_etape3 = true;
    m_etape2 = false;

    m_textAttendreConnection->setVisible(false);

    m_textPresentation->setText(m_textPresentation3.c_str());
    m_lbEquipeBleu->setVisible(true);
    m_lbEquipeRouge->setVisible(true);
    m_bDemarerPartie->setVisible(true);
}

void MyEventReceiver::arreterServeur()
{
    m_textInfoNbJoueurs->setVisible(true);
    m_textAttendreConnection->setVisible(false);
    m_lbEquipeBleu->setVisible(false);
    m_lbEquipeRouge->setVisible(false);
    m_lbClientsConnecte->setVisible(false);
    m_textPresentation->setText(m_textPresentation1.c_str());
    m_ebPort->setVisible(true);
    m_sbNbJoueursAttendu->setVisible(true);
    m_bLancerServeur->setVisible(true);
    m_bAnnuler->setVisible(false);
    m_bDemarerPartie->setVisible(false);
    m_sbNbEliminationGagnePartie->setVisible(true);
    m_textInfo->setText(L"");
    m_lbClientsConnecte->clear();
    m_lbEquipeBleu->clear();
    m_lbEquipeRouge->clear();
    m_etape1 = true;
    m_etape2 = false;
    m_etape3 = false;

    for(unsigned int i = 0; i < m_nbCLientsConnecte; i++)
    {
        m_listeClientsTcp[i].disconnect();
    }

    m_listeCLients.clear();

    m_listener.close();

    m_nbCLientsConnecte = 0;
    m_nbJoueursAttendu = 0;
}

void MyEventReceiver::verifierSiClientDeconnecte()
{
    for(unsigned int i = 0; i < m_nbCLientsConnecte; ++i)
    {
        sf::Packet packet;
        if(std::find(m_listeClientDeconnecte.begin(), m_listeClientDeconnecte.end(), i) == m_listeClientDeconnecte.end())
        {
            if(m_listeClientsTcp[i].receive(packet) == sf::Socket::Disconnected)
            {
                sf::String pseudo = m_listeCLients[i].pseudo;
                std::wstring wstr = pseudo.toWideString();
                wstr += L" est innactif ( il s'est deconnecté )";
                std::wstring text_dejaLa = m_textInfo->getText();
                text_dejaLa += L"\n";
                text_dejaLa += wstr;
                m_textInfo->setText(text_dejaLa.c_str());

                m_listeClientDeconnecte.push_back(i);
            }
        }
    }
}

unsigned short MyEventReceiver::getPort()
{
    return m_port;
}

std::vector<Client> MyEventReceiver::getListeClients()
{
    return m_listeCLients;
}

void MyEventReceiver::creerEquipe()
{
    for(unsigned int i = 0; i<m_lbEquipeBleu->getItemCount(); ++i)
    {
        std::wstring wstr = m_lbEquipeBleu->getListItem(i);
        sf::String str = wstr;
        for(std::vector<Client>::iterator it = m_listeCLients.begin(); it != m_listeCLients.end(); ++it)
        {
            if(str == (*it).pseudo)
            {
                (*it).equipe = true;
            }
        }

        s32 posX_joueurBleu = 1000, posX_joueurRouge = 10000;
        for(std::vector<Client>::iterator it = m_listeCLients.begin(); it != m_listeCLients.end(); ++it)
        {
            if((*it).equipe)
            {
                (*it).positionDepart = posX_joueurBleu;
                posX_joueurBleu -= 500;
            }

            else
            {
                (*it).positionDepart = posX_joueurRouge;
                posX_joueurRouge -= 500;
            }
        }

    }
}

MyEventReceiver::~MyEventReceiver()
{
    m_listener.close();
}
