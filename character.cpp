#include "character.h"


const character Epsilon("$");
const production EpsilonProduction(Epsilon, Epsilon);
const character Border("#");

std::ostream& operator<<(std::ostream& os, const production& prod)
{
    return prod.print(os);
}

std::ostream& operator<<(std::ostream& os, const character& ch)
{
    return ch.print(os);
}


bool character::equal(const character& ch) const
{
    return ch.c == this->c;
}

void character::assign(const character& ch)
{
    this->c = ch.c;
}

character::character(characterType ch)
{
    this->c = ch;
}

character::character()
{
    this->c = EOF;
}

bool character::operator<(const character& p) const
{
    return this->c < p.c;
}
std::ostream& character::print(std::ostream& os) const
{
    return os << this->c;
}
character& character::append(characterType ch)
{
    this->c.append(ch);
    return *this;
}


production::production(character c, character left)
{
    this->clear();
    this->append(c);
    this->m_left = left;
}   

production& production::append(character c)
{
    this->m_right.push_back(c);
    this->m_size++;
    return *this;
}

production& production::append(const production& p)
{
    this->m_left = p.m_left;
    this->m_right.insert(this->m_right.end(), p.m_right.begin(), p.m_right.end());
    this->m_size += p.size();
    return *this;
}

// 或取第index位的值
const character* production::getIndexCharacter(size_t index) const
{
    if(index >= this->m_size)
    {
        return nullptr;
    }
    return &this->m_right[index];
}

size_t production::size() const
{
    return this->m_size;
}

bool production::operator<(const production& p) const
{
    size_t i = 0;
    size_t psize = p.size();
    size_t size = this->size();
    while(true)
    {
        // this 为less函数的第一个参数(map插入时新插入的值)， p是map中已经存在的值
        // 相等时返回false
        if(i >= psize)
        {
            return false;
        }
        if(i >= size)
        {
            return true;
        }
        if (this->m_right[i].equal(p.m_right[i]))
        {
            i++;
            continue;
        }
        return this->m_right[i] < p.m_right[i];
    }
}

// 除了下标为 except 之外，复制其他符号
production& production::copy(size_t except, production& dest) const
{
    dest.clear();
    for(auto i = 0; i < this->size(); i++)
    {
        if(i == except)
        {
            continue;
        }
        dest.append(this->m_right[i]);
    }
    return dest;
}

std::ostream& production::print(std::ostream& os) const
{
    os << "size(" << this->size() << ")";
    for (auto ch : this->m_right)
    {
        os << ch;
    }
    return os;
}

production production::operator+(const production& prod) const
{
    production tmp;
    tmp.m_left = this->m_left;
    tmp.m_right.insert(tmp.m_right.end(), this->m_right.begin(), this->m_right.end());
    tmp.m_right.insert(tmp.m_right.end(), prod.m_right.begin(), prod.m_right.end());
    tmp.m_size = this->size() + prod.size();
    return tmp;
    
}

void production::assignLeft(const character& ch)
{
    this->m_left = ch;
}

void production::clear()
{
    this->m_size = 0;
    this->m_left = Epsilon;
    this->m_right.clear();
}

production::production()
{
    this->clear();
}

bool production::equal(const production& p) const
{
    if(this->m_size != p.m_size)
    {
        return false;
    }
    auto pright = p.m_right;
    for (size_t i = 0; i < this->m_size; i++)
    {
        if (!this->m_right[i].equal(pright[i]))
        {
            return false;
        }
    }
    return true;
}

production production::replace(const character& src, const character& des)
{
    production tmp;
    tmp.m_left = this->m_left;
    auto size = this->m_size;
    for(auto i = 0; i < size; i++)
    {
        if(this->getIndexCharacter(i)->equal(src))
        {
            tmp.append(des);
            continue;
        }
        tmp.append(*this->getIndexCharacter(i));
    }
    return tmp;
}

void production::replace(const character& ch, const int index)
{
    this->m_right.at(index).assign(ch);
    return;
}