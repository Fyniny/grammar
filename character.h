#include <set>
#include <vector>
#include <ostream>
#include <string>

#ifndef CHARACTER_H
#define CHARACTER_H
typedef std::string characterType;

// 单词符号
class character {
    private:
        characterType c;
    public:
        bool equal(const character& ch) const;
        // 给character重新赋值
        void assign(const character& ch);
        character(characterType ch);
        character();

        // 为了满足set插入需要，less函数
        bool operator<(const character& p) const;
        std::ostream& print(std::ostream& os) const;
        character& append(characterType ch);
        friend std::ostream& operator<<(std::ostream& os, const character& ch);
        
};

// 产生式
class production {
    private:
        std::vector<character> m_right; 
        character m_left;
        size_t m_size;
    public:
        bool equal(const production& p) const;

    public:
        production(character c, character left);
        production& append(character c);
        production& append(const production& p);
        // 获取第index位的值
        const character* getIndexCharacter(size_t index) const;
        size_t size() const;
        bool operator<(const production& p) const;
        // 除了下标为 except 之外，复制其他符号
        production& copy(size_t except, production& dest) const;
        // 将串中的src字符替换为des，并返回转化后的串，原串不受影响
        production replace(const character& src, const character& des);
        // 在原串中index序号的字符改为ch字符
        void replace(const character& ch, const int index);
        std::ostream& print(std::ostream& os) const;
        // 追加并返回新串
        production operator+(const production& prod) const;
        // 修改左部
        void assignLeft(const character& ch);
        void clear();
        production();
        friend std::ostream& operator<<(std::ostream& os, const production& prod);
};


// 空字符
extern const character Epsilon; 
// # 号串
extern const character Border; 
// 空串
extern const production EpsilonProduction;
#endif