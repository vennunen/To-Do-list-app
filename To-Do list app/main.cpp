#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip> // For width formatting

namespace todo
{

    // Abstract base class for tasks
    class TaskBase
    {
    public:
        // Pure virtual methods so that derived classes implement these
        virtual void display() const = 0;
        virtual std::string toFileString() const = 0;
        virtual std::string getTitle() const = 0;
        virtual std::string getDeadline() const = 0;
        virtual std::string getCategory() const = 0;
        virtual bool isCompleted() const = 0;
        virtual void markCompleted() = 0;
        virtual ~TaskBase() {} // Virtual destructor for safe polymorphic deletion
    };

    // Concrete task class
    class Task : public TaskBase
    {
    protected:
        std::string title;
        std::string deadline;
        bool completed;

    public:
        // Default constructor
        Task() : title(""), deadline(""), completed(false) {}

        // Parameterized constructor
        Task(const std::string &t, const std::string &d, bool c = false)
            : title(t), deadline(d), completed(c) {}

        // Display task details
        virtual void display() const override
        {
            std::cout << (completed ? "[X] " : "[ ] ")
                      << std::left << std::setw(20) << title
                      << " | Due: " << std::setw(12) << deadline << std::endl;
        }

        // Convert task details to a string for saving
        virtual std::string toFileString() const override
        {
            return title + ";" + deadline + ";" + (completed ? "1" : "0");
        }

        // Mark task as completed
        virtual void markCompleted() override { completed = true; }
        virtual bool isCompleted() const override { return completed; }
        virtual std::string getTitle() const override { return title; }
        virtual std::string getDeadline() const override { return deadline; }
        virtual std::string getCategory() const override { return ""; }

        // Overload << operator to display task
        friend std::ostream &operator<<(std::ostream &os, const Task &task)
        {
            os << (task.completed ? "[X] " : "[ ] ") << task.title << " (Due: " << task.deadline << ")";
            return os;
        }

        // Overload + operator to combine tasks
        Task operator+(const Task &other)
        {
            return Task(title + " & " + other.title, deadline);
        }
    };

    // Derived class with category support
    class CategorizedTask : public Task
    {
    private:
        std::string category;

    public:
        CategorizedTask() : Task(), category("") {}
        CategorizedTask(const std::string &t, const std::string &d, const std::string &cat, bool c = false)
            : Task(t, d, c), category(cat) {}

        // Display including category
        void display() const override
        {
            std::cout << (completed ? "[X] " : "[ ] ")
                      << std::left << std::setw(20) << title
                      << " | Due: " << std::setw(12) << deadline
                      << " | Category: " << category << std::endl;
        }

        // Convert to file string with category
        std::string toFileString() const override
        {
            return title + ";" + deadline + ";" + (completed ? "1" : "0") + ";" + category;
        }

        std::string getCategory() const override { return category; }
    };

    // Task manager class that handles all task operations
    class TaskManager
    {
    private:
        std::vector<TaskBase *> tasks;              // Active tasks
        std::vector<TaskBase *> completedTasks;     // Completed tasks
        std::map<std::string, TaskBase *> titleMap; // Map for quick title lookup
        std::set<std::string> categories;           // Set of all unique categories

        // Convert date string "dd.mm.yyyy" to int "yyyymmdd" for sorting
        static int dateToInt(const std::string &date)
        {
            std::istringstream ss(date);
            std::string day, month, year;
            getline(ss, day, '.');
            getline(ss, month, '.');
            getline(ss, year, '.');
            if (day.length() == 1)
                day = "0" + day;
            if (month.length() == 1)
                month = "0" + month;
            return std::stoi(year + month + day);
        }

    public:
        // Destructor to clean up all dynamically allocated tasks
        ~TaskManager()
        {
            for (auto t : tasks)
                delete t;
            for (auto t : completedTasks)
                delete t;
        }

        // Add a task to the system
        void addTask(TaskBase *task)
        {
            tasks.push_back(task);
            titleMap[task->getTitle()] = task;
            if (!task->getCategory().empty())
                categories.insert(task->getCategory());
        }

        // Display tasks, optionally sorted by deadline
        void viewTasks(bool sorted = false) const
        {
            std::vector<TaskBase *> temp = tasks;
            if (sorted)
            {
                std::sort(temp.begin(), temp.end(), [](TaskBase *a, TaskBase *b)
                          { return dateToInt(a->getDeadline()) < dateToInt(b->getDeadline()); });
            }
            for (const auto &t : temp)
                t->display();
        }

        // Display all completed tasks
        void viewCompleted() const
        {
            for (const auto &t : completedTasks)
                t->display();
        }

        // Mark task as completed by title
        void markCompleted(const std::string &title)
        {
            auto it = titleMap.find(title);
            if (it != titleMap.end())
            {
                it->second->markCompleted();
                completedTasks.push_back(it->second);
                tasks.erase(std::remove(tasks.begin(), tasks.end(), it->second), tasks.end());
                titleMap.erase(it);
            }
        }

        // Delete a task by title
        void deleteTask(const std::string &title)
        {
            auto it = titleMap.find(title);
            if (it != titleMap.end())
            {
                TaskBase *task = it->second;
                tasks.erase(std::remove(tasks.begin(), tasks.end(), task), tasks.end());
                delete task;
                titleMap.erase(it);
            }
        }

        // Search for a task by keyword
        void searchTask(const std::string &query) const
        {
            for (const auto &t : tasks)
            {
                if (t->getTitle().find(query) != std::string::npos)
                {
                    t->display();
                }
            }
        }

        // Filter tasks by category
        void filterByCategory(const std::string &category) const
        {
            std::cout << "Available categories to choose from:\n";
            for (const auto &cat : categories)
            {
                std::cout << " - " << cat << std::endl;
            }
            std::cout << "\nShowing tasks for category: " << category << "\n";
            for (const auto &t : tasks)
            {
                if (t->getCategory() == category)
                    t->display();
            }
        }

        // List all unique categories
        void listAllCategories() const
        {
            std::cout << "Available categories:\n";
            for (const auto &cat : categories)
            {
                std::cout << " - " << cat << std::endl;
            }
        }

        // Save current tasks to file
        void saveToFile(const std::string &filename)
        {
            std::ofstream ofs(filename);
            for (const auto &t : tasks)
                ofs << t->toFileString() << std::endl;
            for (const auto &t : completedTasks)
                ofs << "DONE:" << t->toFileString() << std::endl;
        }

        // Load tasks from file
        void loadFromFile(const std::string &filename)
        {
            std::ifstream ifs(filename);
            std::string line;
            while (getline(ifs, line))
            {
                bool isDone = false;
                if (line.rfind("DONE:", 0) == 0)
                {
                    isDone = true;
                    line = line.substr(5);
                }
                std::stringstream ss(line);
                std::string title, deadline, completedStr, category;

                getline(ss, title, ';');
                getline(ss, deadline, ';');
                getline(ss, completedStr, ';');
                bool completed = (completedStr == "1");

                if (getline(ss, category, ';'))
                {
                    TaskBase *t = new CategorizedTask(title, deadline, category, completed);
                    if (isDone)
                        completedTasks.push_back(t);
                    else
                        addTask(t);
                }
                else
                {
                    TaskBase *t = new Task(title, deadline, completed);
                    if (isDone)
                        completedTasks.push_back(t);
                    else
                        addTask(t);
                }
            }
        }
    };

} // namespace todo

int main()
{
    using namespace todo;
    TaskManager manager;
    manager.loadFromFile("tasks.txt"); // Load saved tasks from file

    int choice;
    do
    {
        // Display menu
        std::cout << "\n--- To-Do List Main Menu ---\n";
        std::cout << "1. Add Task\n2. View Tasks\n3. View Tasks Sorted by Deadline\n4. Mark Task Completed\n5. Delete Task\n6. View Completed\n7. Search Tasks\n8. Filter by Category\n9. List Categories\n0. Exit\nChoice: ";
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1) // Add task
        {
            std::string title, deadline, category;
            std::cout << "Enter title: ";
            getline(std::cin, title);
            std::cout << "Enter deadline (DD.MM.YYYY): ";
            getline(std::cin, deadline);
            std::cout << "Enter category (leave empty for none): ";
            getline(std::cin, category);
            if (category.empty())
                manager.addTask(new Task(title, deadline));
            else
                manager.addTask(new CategorizedTask(title, deadline, category));
        }
        else if (choice == 2) // View tasks
        {
            manager.viewTasks(false);
        }
        else if (choice == 3) // View sorted by deadline
        {
            manager.viewTasks(true);
        }
        else if (choice == 4) // Mark task completed
        {
            std::string title;
            std::cout << "Enter title to mark completed: ";
            getline(std::cin, title);
            manager.markCompleted(title);
        }
        else if (choice == 5) // Delete task
        {
            std::string title;
            std::cout << "Enter title to delete: ";
            getline(std::cin, title);
            manager.deleteTask(title);
        }
        else if (choice == 6) // View completed
        {
            manager.viewCompleted();
        }
        else if (choice == 7) // Search
        {
            std::string query;
            std::cout << "Enter title keyword to search: ";
            getline(std::cin, query);
            manager.searchTask(query);
        }
        else if (choice == 8) // Filter by category
        {
            std::string category;
            manager.listAllCategories();
            std::cout << "Enter category to filter by: ";
            getline(std::cin, category);
            manager.filterByCategory(category);
        }
        else if (choice == 9) // Show saved categories
        {
            manager.listAllCategories();
        }

    } while (choice != 0);

    manager.saveToFile("tasks.txt"); // Save tasks before exit
    return 0;
}
