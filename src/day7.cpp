/******************************************************************************
 * File:        day7.cpp
 *
 * Author:      yut23
 * Created:     2022-12-07
 *****************************************************************************/

#include "lib.h"
#include <algorithm>
#include <memory>
#include <numeric>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class Entity : public std::enable_shared_from_this<Entity> {
  private:
    Entity() = default;

  protected:
    Entity(const std::string &name, int size = 0) : name(name), size(size){};

  public:
    std::string name{};
    int size;

    virtual ~Entity() = default;

    std::shared_ptr<Entity> getptr() { return shared_from_this(); }

    // pure virtual function makes this an abstract class
    virtual void pretty_print(std::ostream &,
                              const std::string &indent = "") const = 0;
};

std::ostream &operator<<(std::ostream &os, const Entity &e) {
    os << e.name;
    return os;
}

class File : public Entity {
  public:
    File(const std::string &name, int size) : Entity(name, size) {}
    ~File() = default;

    void pretty_print(std::ostream &, const std::string &indent) const override;
};

void File::pretty_print(std::ostream &os, const std::string &indent) const {
    os << indent << "- " << *this << " (file, size=" << size << ")"
       << std::endl;
}

/// Stores entities in insertion order. Duplicate names are forbidden.
class Directory : public Entity {
  private:
    std::unordered_map<std::string, int> entity_index{};
    std::vector<std::shared_ptr<Entity>> contents{};
    // use a weak reference here to prevent cycles
    std::weak_ptr<Directory> weak_parent{};

  public:
    explicit Directory(const std::string &name) : Entity(name) {}
    Directory(const std::string &name, std::shared_ptr<Directory> parent)
        : Entity(name), weak_parent(parent) {}

    void insert(std::shared_ptr<Entity> ent);
    std::shared_ptr<Directory> get_subdir(const std::string &name);

    void pretty_print(std::ostream &,
                      const std::string &indent = "") const override;
};

void Directory::insert(std::shared_ptr<Entity> ent) {
    // try adding the destination index to the lookup
    if (!entity_index.try_emplace(ent->name, contents.size()).second) {
        throw std::logic_error{"Tried to insert a duplicate entity"};
    }
    contents.push_back(ent);
    // update size of this directory and all parent directories up the tree
    std::shared_ptr<Directory> dir =
        std::dynamic_pointer_cast<Directory>(getptr());
    dir->size += ent->size;
    while ((dir = dir->weak_parent.lock())) {
        dir->size += ent->size;
    }
}

std::shared_ptr<Directory> Directory::get_subdir(const std::string &name) {
    return std::dynamic_pointer_cast<Directory>(
        contents[entity_index.at(name)]);
}

void Directory::pretty_print(std::ostream &os,
                             const std::string &indent) const {
    os << indent << "- " << *this << " (dir)" << std::endl;
    auto new_indent = indent + "  ";
    for (const auto &ent : contents) {
        ent->pretty_print(os, new_indent);
    }
}

int main(int argc, char **argv) {
    auto infile = parse_args(argc, argv);

    std::shared_ptr<Directory> root = std::make_shared<Directory>("/");
    std::vector<std::shared_ptr<Directory>> all_dirs{root};
    std::stack<std::shared_ptr<Directory>> dirstack;
    dirstack.push(root);
    // read file line-by-line
    std::string line;
    while (std::getline(infile, line)) {
        std::stringstream ss{line};
        if (line[0] == '$') { // command
            char ch;
            std::string cmd;
            ss >> ch >> cmd;
            if (cmd == "cd") {
                std::string dest;
                ss >> dest;
                if (dest == "..") {
                    dirstack.pop();
                } else if (dest == "/") {
                    // ascend back to root
                    while (dirstack.top() != root) {
                        dirstack.pop();
                    }
                } else {
                    dirstack.push(dirstack.top()->get_subdir(dest));
                }
            } else {
                assert(cmd == "ls");
                // ignore command
            }
        } else { // ls output
            std::string first, name;
            ss >> first >> name;
            if (first == "dir") {
                auto dir = std::make_shared<Directory>(name, dirstack.top());
                dirstack.top()->insert(dir);
                all_dirs.push_back(dir);
            } else {
                int size = std::stoi(first);
                auto file = std::make_shared<File>(name, size);
                dirstack.top()->insert(file);
            }
        }
    }

#ifdef DEBUG_MODE
    root->pretty_print(std::cerr);
#endif

    constexpr int total_space = 70000000;
    constexpr int required_space = 30000000;
    const int min_to_delete = required_space - (total_space - root->size);

    int part_1_total = 0;
    int part_2_min = root->size;
    for (const auto &dir : all_dirs) {
        int size = dir->size;
        // for part 1, we only want the directories with a total size of at most
        // 100,000
        if (size <= 100000) {
            part_1_total += size;
        }
        if (size >= min_to_delete && size < part_2_min) {
            part_2_min = size;
        }
    }
    std::cout << part_1_total << std::endl;
    std::cout << part_2_min << std::endl;
    return EXIT_SUCCESS;
}
