#include <stdio.h>
#include <vector>
#include <string>
#include "filesystem.h"
#include <memory>

struct uuid_file
{
    FILE *text_file = nullptr;
    std::string last_readed;

    uuid_file(const std::string &file);
    ~uuid_file();
    bool next();


    uuid_file(const uuid_file &other) = delete;
    uuid_file &operator =(const uuid_file &other) = delete;
};

bool lesser_comparator(const std::string &s1, const std::string &s2)
{
    return s1 < s2;
}

bool greater_comparator(const std::string &s1, const std::string &s2)
{
    return s1 > s2;
}

int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        printf("usage: uuidpage <directory> <page> [page_size]\n");
        return 0;
    }
    int n = 0;
    sscanf(argv[2], "%d", &n);
    int page_size = 500;
    if(argc == 4)
    {
        sscanf(argv[3], "%d", &page_size);
    }
    std::string dir = argv[1];

    std::vector<std::string> files;
    getFiles(dir, files);


    bool inc;
    {
        //определяем направление сортировки
        uuid_file t("./"+dir+"/"+files[0]);
        std::string first = t.last_readed;
        t.next();
        std::string second = t.last_readed;
        inc = first < second;
        inc ? printf("increasing\n") : printf("decreasing\n");
    }
    bool (*comparator)(const std::string &s1, const std::string &s2) = inc ? greater_comparator : lesser_comparator;

    std::vector<std::unique_ptr<uuid_file>> all_uuids;
    for(auto &a : files)
    {
        all_uuids.push_back(std::unique_ptr<uuid_file>(new uuid_file("./"+dir+"/"+a)));
    }

    int from_top_counter = 0;
    std::string bottom_mark;
    int iterations = 0;
    bool end = false;

    while (from_top_counter < (n+1)*page_size && !end)
    {
        bottom_mark = all_uuids[0]->last_readed; //выставляем нижнюю метку

        int bottom_try_winner = 0;                   //номер файла, в котором располается следующий uuid
        for(size_t i = 1; i < all_uuids.size(); ++i) //ищем возможные повышения нижней метки в остальных файлах
        {
            if(!(*comparator)(all_uuids[i]->last_readed, bottom_mark))
            {
                bottom_mark = all_uuids[i]->last_readed;
                bottom_try_winner = i;
            }
            iterations++;
        }

        if(!all_uuids[bottom_try_winner]->next()) //смещаем положение файла, который хранил верхнюю метку
        {                                        //если смещение не удалось, то файл кончился, удаляем его
            std::swap(all_uuids[all_uuids.size() - 1], all_uuids[bottom_try_winner]);
            all_uuids.pop_back();
            if(all_uuids.size() == 0)            //файлы кончились
                end = true;
        }

        ++from_top_counter;
        if(from_top_counter > n*page_size)
        {
            printf("%s\n", bottom_mark.c_str());
        }
    }

    printf("iter:%d count:%d\n", iterations, from_top_counter);

    return 0;
}

uuid_file::uuid_file(const std::string &file)
{
    text_file = fopen(file.c_str(), "r");
    last_readed.resize(36);

    fscanf(text_file, "%s\n", &last_readed[0]);
}

uuid_file::~uuid_file()
{
    if(text_file)
    {
        fclose(text_file);
        text_file = nullptr;
    }
}

bool uuid_file::next()
{
    fscanf(text_file, "%s\n", &last_readed[0]);
    return !feof(text_file);
}

