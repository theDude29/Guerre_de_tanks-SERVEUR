/* Guerre de Tank, bataille de tank en réseau. Copyright (C) 2018 Rémi Pérenne.
Ce programme est un logiciel libre ; vous pouvez le redistribuer et/ou le modifier au titre des clauses de la Licence Publique Générale GNU, telle que publiée par la Free Software Foundation ; soit la version 2 de la Licence, ou (à votre discrétion) une version ultérieure quelconque. Ce programme est distribué dans l'espoir qu'il sera utile, mais SANS AUCUNE GARANTIE ; sans même une garantie implicite de COMMERCIABILITE ou DE CONFORMITE A UNE UTILISATION PARTICULIERE. Voir la Licence Publique Générale GNU pour plus de détails. Vous devriez avoir reçu un exemplaire de la Licence Publique Générale GNU avec ce programme ; si ce n'est pas le cas, écrivez à la Free Software Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#include "MyEventReceiver.h"

using namespace irr;

int main()
{
    //on initialise les nombres aleatoire
    srand(time(NULL));

    //Parties importantes de irrlicht
    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(1000,600), 32);
    video::IVideoDriver* driver = device->getVideoDriver();
    gui::IGUIEnvironment* gui = device->getGUIEnvironment();
    //On change la police
    gui::IGUISkin *skin = gui->getSkin();
    skin->setFont(gui->getFont("fonthaettenschweiler.bmp"));
    //on met un titre
    device->setWindowCaption(L"Guerre de tank - Serveur");

    video::ITexture *titre, *sfml, *irrlicht;
    titre = driver->getTexture("titre.png");
    driver->makeColorKeyTexture(titre, core::position2d<s32>(0,0));
    sfml = driver->getTexture("logosfml.png");
    driver->makeColorKeyTexture(sfml, core::position2d<s32>(0,0));
    irrlicht = driver->getTexture("logoirrlicht.png");

    while(device->run())
    {
        //event receiver
        MyEventReceiver* receiver = new MyEventReceiver(gui, device->getTimer(), driver);
        device->setEventReceiver(receiver);

        core::dimension2du screenSize;
        bool partieCommence = false;
        while(device->run() && !partieCommence)
        {
            driver->beginScene(true, true, video::SColor(255, 100, 100, 100));

            screenSize = driver->getScreenSize();
            driver->draw2DImage(titre, core::position2di(screenSize.Width/2 - 340, screenSize.Height/2 - 250), core::rect<s32>(0, 0, 550, 100), 0, video::SColor(255,255,255,255), true);
            driver->draw2DImage(irrlicht, core::position2di(30, screenSize.Height - 80));
            driver->draw2DImage(sfml, core::position2di(screenSize.Width - 300, screenSize.Height - 100), core::rect<s32>(0, 0, 270, 80), 0, video::SColor(255,255,255,255), true);
            partieCommence = receiver->miseAJour(screenSize);

            gui->drawAll();

            driver->endScene();
        }

        sf::UdpSocket socket;
        socket.bind(receiver->getPort());
        socket.setBlocking(false);

        std::vector<Client> listeClients = receiver->getListeClients();
        std::vector<Client>::iterator listeClients_it = listeClients.begin();
        std::vector<Client>::iterator listeClients_itEnd = listeClients.end();

        sf::Packet packet;
        sf::IpAddress adresseIP;
        unsigned short port;

        irr::gui::IGUIStaticText* text = gui->addStaticText(L"La partie a commencée.", core::rect<s32>(0,0,250,30));

        bool finDeLaPartie = false;
        while(device->run() && !finDeLaPartie)
        {
            driver->beginScene(true, true, video::SColor(255, 100, 100, 100));
            screenSize = driver->getScreenSize();
            text->setRelativePosition(core::position2di(screenSize.Width/2 - 100, screenSize.Height/2 - 30));
            driver->draw2DImage(titre, core::position2di(screenSize.Width/2 - 340, screenSize.Height/2 - 250), core::rect<s32>(0, 0, 550, 100), 0, video::SColor(255,255,255,255), true);
            driver->draw2DImage(irrlicht, core::position2di(30, screenSize.Height - 80));
            driver->draw2DImage(sfml, core::position2di(screenSize.Width - 300, screenSize.Height - 100), core::rect<s32>(0, 0, 270, 80), 0, video::SColor(255,255,255,255), true);
            gui->drawAll();
            driver->endScene();

            //quant on recoit un message on verifie simplement
            //si il est vide (c'est le signal pour dire que la partie
            //est terminé) sinon on se contente de le renvoyer a tous
            //les clients et ils se demmerde avec
            if(socket.receive(packet, adresseIP, port) == sf::Socket::Done)
            {
                if(packet.getDataSize() == 0)
                    finDeLaPartie = true;
                else
                {
                    for(listeClients_it = listeClients.begin(); listeClients_it != listeClients_itEnd; listeClients_it++)
                    {
                        socket.send(packet, (*listeClients_it).adresse, (*listeClients_it).port);
                    }
                }
            }
        }

        socket.unbind();
        delete receiver;

        text->remove();
    }
    device->drop();

    return 0;
}
