#include "Editor.hpp"
#include "edit.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "TextBoxStream.hpp"
#include "Scene.hpp"
#include <fstream>

#include <iostream>



/*#ifdef _WIN32
#include <sstream>

namespace std
{

std::string to_string(std::size_t nb)
{
	std::stringstream ss;
	ss<< nb;
	return ss.str();
}


}
#endif*/


namespace edit
{
	sf::Font Editor::stdFont;

	Editor::Editor(std::size_t& number, sf::Music& voice, sf::Music& music)
	{
		m_curCharacter = 0;
		m_characters.push_back(CharacterInfo(L"", Character()));
		m_characters[0].second.setSlot(1, 1);

		m_displayer.setFont(stdFont);
		m_displayer.setColor(sf::Color::White);
		m_displayer.setString("mode: \nvoice: \nmusic: ");

		m_characterDisplayer.setFont(stdFont);
		m_characterDisplayer.setColor(sf::Color::White);
		updateCharacterDisplayer();

		m_currentItem = Action::invalidItem;

		m_characterVertex.resize(5);
		m_characterVertex.setPrimitiveType(sf::LinesStrip);
		for(std::size_t i = 0; i < m_characterVertex.getVertexCount(); i++)
		{
			m_characterVertex[i].color = sf::Color::Red;
		}

		updateCharacterVertex();

		m_voice = &voice;
		m_music = &music;

		m_number = &number;
	}




	/* Editor::Editor(const Editor& editor) */
	/* { */
	/* 	m_currentItem = editor.m_currentItem; */
	/* 	m_characters = editor.m_characters; */
	/* 	m_curCharacter = editor.m_curCharacter; */
	/* 	m_text = editor.m_text; */
	/* 	m_voice = editor.m_voice; */
	/* 	m_voiceFile = editor.m_voiceFile; */
	/* 	m_music = editor.m_music; */
	/* 	m_musicFile = editor.m_musicFile; */
	/* 	m_displayer = editor.m_displayer; */
	/* 	m_characterVertex = editor.m_characterVertex; */
	/* } */


	void Editor::loadStaticData()
	{
		if(!stdFont.loadFromFile(FONT_PATH + std::string("CANON.ttf")))
		{
			std::cerr<< "Unable to load CANON.ttf\n";
		}
	}





	void Editor::setNumber(std::size_t &number)
	{
		m_number = &number;
	}




	void Editor::handleAction(Action &action)
	{
		m_currentItem = action.aItem;
		switch(action.aItem)
		{
			case Action::character:
				handleCharacter(action);
				break;
			case Action::text:
				handleText(action);
				break;
			case Action::voice:
				handleVoice(action);
				break;
			case Action::music:
				handleMusic(action);
				break;
			case Action::background:
				handleBackground(action);
				break;
			default:
				break;
		}


	}



	void Editor::handleCharacter(Action &action)
	{
		//select the Character / slot we want
		bool slotsNeedsUpdates = false;
		while(action.h > 0)
		{
			m_curCharacter++;
			if(m_curCharacter >= m_characters.size())
			{
				m_characters.push_back(CharacterInfo(L"", Character()));
				slotsNeedsUpdates = true;
			}
			action.h--;
		}
		while(action.h < 0)
		{
			if(m_curCharacter == 0)
			{
				if(!m_characters.empty())
				{
					m_characters.pop_back();
					slotsNeedsUpdates = true;
				}
				else
				{
					m_curCharacter--;
				}
			}
			else
			{
				m_curCharacter--;
			}
			
			action.h++;
		}
		if(slotsNeedsUpdates)
		{
			for(size_t i = 0; i < m_characters.size(); i++)
			{
				m_characters[i].second.setSlot(i+1, m_characters.size());
			}
		}

		if(action.v > 0)
		{	
			//aliases to simplify the reading
			std::wstring &curFile(m_characters[m_curCharacter].first);
			Character &curCharacter(m_characters[m_curCharacter].second);


			curFile = selectFile(L"Select a character file");
			//curFile = CHARACTER_PATH + m_characters[m_curCharacter].first;

			curCharacter.setTexture(Scene::requestCharacterTexture(curFile));
		}
	}

	void Editor::handleText(Action &action)
	{
		if(!action.textBuffer.isEmpty())
		{
			//std::cout<< "unsigned int : "<< static_cast<unsigned int>(action.textBuffer[0])<< "\n";


			for(std::size_t i = 0; i < action.textBuffer.getSize(); i++)
			{
				wchar_t c = action.textBuffer[i];
				switch(c)
				{
					case 8: //delete
					{
						if(!m_text.empty())
						{
							m_text.pop_back();
						}
						break;
					}
					case 27: //echap
						break;
					case 13: //enter
						m_text += L'\n';
						break;
					default:
						m_text += action.textBuffer[i];
						break;

				}
			}

			//std::locale::global(std::locale(""));
			//std::wcout.imbue(std::locale(""));
			//std::wcout<< action.textBuffer.toWideString()<< "\n";
		}
	}

	void Editor::handleVoice(Action &action)
	{
		if(action.v > 0)
		{
			m_voiceFile = selectFile(L"Select a voice file");
			m_voiceFile = VOICE_PATH + m_voiceFile;
			std::string file(m_voiceFile.begin(), m_voiceFile.end());
			m_voice->stop();
			if(m_voice->openFromFile(file))
				m_voice->play();
		}
		else if(action.v < 0)
		{
			m_voice->stop();
			m_voiceFile.clear();
		}
	}

	void Editor::handleMusic(Action &action)
	{
		if(action.v > 0)
		{
			m_musicFile = selectFile(L"Select a music file");
			m_musicFile = MUSIC_PATH + m_musicFile;
			std::string file(m_musicFile.begin(), m_musicFile.end());
			m_music->stop();
			if(m_music->openFromFile(file))
				m_music->play();
				m_music->setLoop(true);
		}
		else if(action.v < 0)
		{
			m_music->stop();
			m_musicFile.clear();
		}
	}


	void Editor::handleBackground(Action& action)
	{
		if(action.v > 0)
		{
			m_bgFile = selectFile(L"Select a background file");
			m_bg.setTexture(Scene::requestBgTexture(m_bgFile));
		}
	}


	void Editor::updateDisplayers()
	{
		updateCharacterVertex();
		updateCharacterDisplayer();

		sf::String displayerString;
		displayerString += "editor number: ";
		displayerString += std::to_string(*m_number);
		displayerString += "\n";
		displayerString += "mode: ";
		displayerString += toString(m_currentItem);
		displayerString += "\n";
		displayerString += "voice: ";
		displayerString += m_voiceFile;
		displayerString += "\n";
		displayerString += "music: ";
		displayerString += m_musicFile;
		displayerString += "\n";
		displayerString += "background: ";
		displayerString += m_bgFile;

		m_displayer.setString(displayerString);
	}


	void Editor::updateCharacterVertex()
	{
		//verify that we are correctly selecting a character
		if(m_characters.size() - m_curCharacter > m_characters.size())
		{
			return;
		}
		

		sf::FloatRect bounds;
		bounds = m_characters[m_curCharacter].second.getGlobalBounds();

		//If there is no sprite : draw a vertical line to indicate what slot is currently selected
		if(bounds.width <= 1.)
		{
			m_characterVertex[0].position.x = m_characters[m_curCharacter].second.getPosition().x;
			m_characterVertex[0].position.y = 0.;
			
			m_characterVertex[1].position.x = m_characterVertex[0].position.x;
			m_characterVertex[1].position.y = WINDOW_SIZE.y;

			for(std::size_t i = 2; i < m_characterVertex.getVertexCount(); i++)
			{
				m_characterVertex[i].position = m_characterVertex[1].position;
			}

			return;
		}

		m_characterVertex[0].position.x = bounds.left;
		m_characterVertex[0].position.y = bounds.top;

		m_characterVertex[1].position.x = bounds.left + bounds.width;
		m_characterVertex[1].position.y = bounds.top;

		m_characterVertex[2].position.x = bounds.left + bounds.width;
		m_characterVertex[2].position.y = bounds.top + bounds.height;

		m_characterVertex[3].position.x = bounds.left;
		m_characterVertex[3].position.y = bounds.top + bounds.height;

		m_characterVertex[4] = m_characterVertex[0];
	}


	void Editor::updateCharacterDisplayer()
	{
		sf::String displayerString;

		displayerString += "slots: ";
		displayerString += std::to_string(m_characters.size());
		displayerString += '\n';
		displayerString += "current: ";
		displayerString += std::to_string(m_curCharacter + 1);

		m_characterDisplayer.setString(displayerString);

		auto bounds = m_characterDisplayer.getGlobalBounds();
		
		m_characterDisplayer.setOrigin(bounds.width, 0);
		m_characterDisplayer.setPosition(WINDOW_SIZE.x - bounds.width, 0);
	}

	
	void Editor::saveToStream(std::wofstream& stream)
	{
		stream<< L"[\n";
		
		//save the characters
		for(std::size_t i = 0; i < m_characters.size(); i++)
		{
			saveCharacter(i, stream);	
		}

		saveText(stream);
		saveVoice(stream);
		saveMusic(stream);
		saveBackground(stream);

		stream<< L"]\n";
	}


	void Editor::saveCharacter(std::size_t pos, std::wofstream& stream)
	{
		//reference to easy the reading
		Character& curCharacter(m_characters[pos].second);
		std::wstring curFile(m_characters[pos].first);

		if(!curFile.empty())
		{

			stream<< L"character{";
			
			stream<< curFile;

			auto slots = curCharacter.getSlot();
			if(slots.second != 0) //there are indeed slots
			{
				stream<< L" slot "<< slots.first<< L' '<< slots.second;
			}
			else //write the position instead
			{
				auto curPos = curCharacter.getPosition();
				stream<< L" pos "<< curPos.x<< L' '<< curPos.y;
			}

			stream<< L" }\n";

		}
	}

	void Editor::saveText(std::wofstream& stream)
	{
		stream<< L"text{";
		stream<< L"cl ";

		stream<< cutPath(m_text);

		stream<< L" }\n";
	}

	void Editor::saveVoice(std::wofstream& stream)
	{
		if(!m_voiceFile.empty())
		{
			stream<< L"voice{";

			stream<< cutPath(m_voiceFile);

			stream<< L" }\n";
		}
	}

	void Editor::saveMusic(std::wofstream& stream)
	{
		if(!m_musicFile.empty())
		{
			stream<< L"music{";

			stream<< cutPath(m_musicFile);

			stream<< L" }\n";
		}
	}

	void Editor::saveBackground(std::wofstream& stream)
	{
		if(!m_bgFile.empty())
		{
			stream<< L"background{";

			stream<< m_bgFile;

			stream<< L" }\n";
		}
	}




	void Editor::draw(sf::RenderTarget &target, sf::RenderStates states) const
	{
		target.draw(m_bg);
		for(size_t i = 0; i < m_characters.size(); i++)
		{
			target.draw(m_characters[i].second, states);
		}
		tstream.clear();
		tstream<< m_text;

		target.draw(m_displayer, states);
		target.draw(m_characterDisplayer, states);
		target.draw(m_characterVertex, states);

		//std::cout<< m_curCharacter<< " : "<< m_characters.size()<<"\n";
	}


} //namespace edit


