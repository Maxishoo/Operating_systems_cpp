#include <lib.h>

int main(int argc, char *argv[])
{
    Node I = createNode(atoi(argv[1]), true);
    std::map<std::string, int> dict;

    std::list<Node> children;
    while (true)
    {
        for (auto &i : children)
        {
            message m = get_mes(i);
            if (m.command != None)
                send_mes(I, m);
        }

        // проверяем сообщение от родителя
        message m = get_mes(I);
        switch (m.command)
        {
        case Create:
            if (m.id == I.id) // добавляем себе нового ребенка
            {
                Node child = createProcess(m.num);
                children.push_back(child);
                send_mes(I, {Create, child.id, child.pid});
            }
            else // отправляем команду на добавление ребенка детям
                for (auto &i : children)
                    send_mes(i, m);
            break;
        case Ping:
            if (m.id == I.id)
                send_mes(I, m); // спросили меня, отправляем что мы живы
            else                // отправляем детям запрос
                for (auto &i : children)
                    send_mes(i, m);
            break;
        case ExecAdd:
            if (m.id == I.id) // спросили меня, отправляем ответ
            {
                dict[std::string(m.st)] = m.num;
                send_mes(I, m);
            }
            else // отправляем детям запрос
                for (auto &i : children)
                    send_mes(i, m);
            break;
        case ExecFnd:
            if (m.id == I.id) // спросили меня, отправляем ответ
                if (dict.find(std::string(m.st)) != dict.end())
                    send_mes(I, {ExecFnd, I.id, dict[std::string(m.st)], m.st});
                else
                    send_mes(I, {ExecErr, I.id, -1, m.st});
            else // отправляем детям запрос
                for (auto &i : children)
                    send_mes(i, m);
            break;
        default:
            break;
        }
        usleep(100000);
    }
    return 0;
}