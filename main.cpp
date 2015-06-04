#include <stdio.h>
#include <vector>
#include <string>
#include "filesystem.h"
#include "uuid.h"

struct uuid_file
{
    FILE *text_file = nullptr;
    std::string last_readed;

    uuid_file(){}
    ~uuid_file();
    bool next();
    void open(const std::string &s);
    void close();

    uuid_file(const uuid &val) = delete;
    uuid_file &operator =(uuid &val) = delete;
};

bool lesser_comparator(const std::string &s1, const std::string &s2)
{
    return s1 < s2;
}

bool greater_comparator(const std::string &s1, const std::string &s2)
{
    return s1 > s2;
}

/**
 * @brief place_min_file_on_top Найти файл, содержащий минимальный элемент
 * в текущей позиции и устанавливает его в положение 0 в списке файлов
 *
 * @param all_uuids список файлов uuid
 * @param comparator компаратор, абстрагирующий убывает сортировка или возрастает
 */
void place_min_file_on_top(std::vector<uuid_file> &all_uuids, bool (*comparator)(const std::string &s1, const std::string &s2))
{
    std::string cur = all_uuids[0].last_readed;
    int cur_n = 0;
    for(int i = 1; i < all_uuids.size(); ++i)
    {
        if(!(*comparator)(all_uuids[i].last_readed, cur))
        {
                cur_n = i;
                cur = all_uuids[i].last_readed;
        }
    }
    std::swap(all_uuids[0], all_uuids[cur_n]);
}

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("usage: uuidpage <directory> <page> [page_size]/n");
        return 0;
    }
    int n = 0;
    sscanf(argv[2], "%d", &n);
    int PAGE_SIZE = 500;
    if(argc == 4)
    {
        sscanf(argv[3], "%d", &PAGE_SIZE);
    }
    std::string dir = argv[1];

    std::vector<std::string> files;
    getFiles(dir, files);


    bool inc;
    {
        //определяем направление сортировки
        uuid_file t;
        t.open("./"+dir+"/"+files[0]);
        std::string first = t.last_readed;
        t.next();
        std::string second = t.last_readed;
        inc = first < second;
        t.close();
    }
    bool (*comparator)(const std::string &s1, const std::string &s2) = inc ? greater_comparator : lesser_comparator;

    std::vector<uuid_file> all_uuids;
    for(auto a : files)
    {
        all_uuids.emplace_back();
        all_uuids[all_uuids.size() - 1].open("./"+dir+"/"+a);
    }

    place_min_file_on_top(all_uuids, comparator);

    int from_top_counter = 0;
    int bottom_try_winner = 0; //номер файла, в котором будет располагаться следующий uuid
    std::string top_mark;

    while (from_top_counter < (n+1)*PAGE_SIZE)
    {
        if(bottom_try_winner == 0) { //если нижняя метка была найдена у другом файле, то новую искать в первом файле еще рано
            top_mark = all_uuids[0].last_readed;
            if(!all_uuids[0].next()) //мы дошли до конца первого файла
            {
                if(all_uuids.size() == 1) //это последний файл и он кончится, перебрали всё
                    break;
                place_min_file_on_top(all_uuids, comparator); //ищем новый первый, старый удаляем
                std::swap(all_uuids[0], all_uuids[all_uuids.size() - 1]);
                all_uuids.pop_back();
            }
        }
        std::string bottom_mark = all_uuids[0].last_readed; //выставляем нижнюю метку

        bottom_try_winner = 0;
        for(int i = 1; i < all_uuids.size(); ++i) //ищем возможные повышения верхней метки в остальных файлах
        {
            std::string bottom_try = all_uuids[i].last_readed;
            if(!(*comparator)(bottom_try, bottom_mark))
            {
                bottom_mark = bottom_try;
                bottom_try_winner = i;
            }
        }

        if(bottom_try_winner != 0) {
            if(!all_uuids[bottom_try_winner].next()) //смещаем положение файла, который хранил следующий uuid
            {                                        //если смещение не удалось, то файл кончился, удаляем его
                std::swap(all_uuids[all_uuids.size() - 1], all_uuids[bottom_try_winner]);
                all_uuids.pop_back();
            }
        }
        //найден гарантированно следующий uuid
        ++from_top_counter;
        if(from_top_counter > n*PAGE_SIZE)
        {
            printf("%s\n", top_mark.c_str());
        }
        top_mark = bottom_mark;
    }

    for(auto a : all_uuids)
    {
        a.close();
    }

    return 0;
}

uuid_file::~uuid_file()
{
}

bool uuid_file::next()
{
    fscanf(text_file, "%s\n", &last_readed[0]);
    return !feof(text_file);
}

void uuid_file::open(const std::string &s)
{
    text_file = fopen(s.c_str(), "r");
    last_readed.resize(36);

    fscanf(text_file, "%s\n", &last_readed[0]);
}

void uuid_file::close()
{
    if(text_file)
    {
        fclose(text_file);
        text_file = nullptr;
    }
}
