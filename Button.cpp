#include "Button.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "constants.hpp"
#include "OpenFileError.hpp"


Button::Button(const std::wstring& msg, const std::wstring& expr)
{
	m_expression = expr;
	m_sprite.setFillColor(sf::Color::White);
	initText();
	m_text.setString(msg);
	updateDisplay();
}


sf::Font Button::stdFont;




/* bool Button::wasClicked(const sf::Vector2f& pointer) const */
/* { */
/* 	sf::FloatRect bounds(m_sprite.getGlobalBounds()); */

/* 	return !( */
/* 	pointer.x < bounds.left || */
/* 	pointer.x > bounds.left + bounds.width || */
/* 	pointer.y < bounds.top || */
/* 	pointer.y > bounds.top + bounds.height); */
/* } */



sf::FloatRect Button::getGlobalBounds() const
{
	auto bounds = m_sprite.getGlobalBounds();
	
	return getTransform().transformRect(bounds);
}



void Button::setWidth(float w)
{
	float height = m_sprite.getSize().y;

	m_sprite.setSize(sf::Vector2f(w, height));
}



void Button::setExpression(const std::wstring& expr)
{
	m_expression = expr;
}

std::wstring Button::getExpression() const
{
	return m_expression;
}




void Button::loadStaticData()
{
	if(!stdFont.loadFromFile(FONT_PATH + std::string("nyala.ttf")))
	{
		throw OpenFileError();
	}
}



void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	target.draw(m_sprite, states);
	target.draw(m_text, states);
}





void Button::initText()
{
	m_text.setFont(stdFont);
	m_text.setColor(sf::Color::Black);
}




void Button::updateDisplay()
{
	sf::FloatRect bounds(m_text.getGlobalBounds());

	sf::Vector2f textSize(bounds.width, bounds.height);
	m_sprite.setSize(textSize + sf::Vector2f(30*2, 15*2));

	m_text.setPosition(m_sprite.getPosition() + sf::Vector2f(30, 15));
}





bool collision(const sf::Vector2f& point, const Button& b)
{
	auto bounds = b.getGlobalBounds();

	return !( 
	point.x < bounds.left ||
	point.x > bounds.left + bounds.width ||
	point.y < bounds.top ||
 	point.y > bounds.top + bounds.height);
}

