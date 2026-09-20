// Student Manager
// A console program to add, show, search, update, delete and sort students.
// Data is saved in students.csv, so it is still there after you close the program.

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdlib>
using namespace std;

const string FILE_NAME = "students.csv";
const double MIN_GRADE = 1.0;   // change these two to match your school's grading scale
const double MAX_GRADE = 6.0;

// One student = one struct that keeps all their data together
struct Student
{
    string firstName;
    string lastName;
    string className;
    double average = 0;
};

// ---------- Input helpers ----------

// Ask a question and read a whole line (so names can contain spaces)
string readLine(const string& prompt)
{
    string line;
    cout << prompt;
    if (!getline(cin, line))
        exit(0);   // input was closed, so stop the program
    return line;
}

// Read text that is not empty. The ; character is not allowed
// because we use it to separate the fields in the file.
string readText(const string& prompt)
{
    while (true)
    {
        string text = readLine(prompt);
        if (!text.empty() && text.find(';') == string::npos)
            return text;
        cout << "Please enter some text (without the ; character)." << endl;
    }
}

// Read a whole number. Returns -1 if the input is not a number.
int readInt(const string& prompt)
{
    string line = readLine(prompt);
    try
    {
        size_t used;
        int value = stoi(line, &used);
        if (used == line.size())
            return value;
    }
    catch (...) {}
    return -1;
}

// Keep asking until the user enters a valid average grade
double readAverage(const string& prompt)
{
    while (true)
    {
        string line = readLine(prompt);
        try
        {
            size_t used;
            double value = stod(line, &used);
            if (used == line.size() && value >= MIN_GRADE && value <= MAX_GRADE)
                return value;
        }
        catch (...) {}
        cout << "Please enter a number between " << MIN_GRADE << " and " << MAX_GRADE << "." << endl;
    }
}

// Make text lowercase, so searching ignores capital letters
string toLower(string text)
{
    for (char& c : text)
        c = (char)tolower((unsigned char)c);
    return text;
}

// ---------- File functions ----------

// Read all students from the file into a vector
vector<Student> loadStudents()
{
    vector<Student> students;
    ifstream file(FILE_NAME);
    string line;

    while (getline(file, line))
    {
        stringstream fields(line);
        Student s;
        string average;

        if (getline(fields, s.firstName, ';') &&
            getline(fields, s.lastName, ';') &&
            getline(fields, s.className, ';') &&
            getline(fields, average))
        {
            try
            {
                s.average = stod(average);
                students.push_back(s);
            }
            catch (...) {}   // skip broken lines
        }
    }
    return students;
}

// Write all students to the file (replaces the old content)
void saveStudents(const vector<Student>& students)
{
    ofstream file(FILE_NAME);
    for (const Student& s : students)
        file << s.firstName << ";" << s.lastName << ";" << s.className << ";" << s.average << "\n";
}

// ---------- Display ----------

void printStudents(const vector<Student>& students)
{
    if (students.empty())
    {
        cout << "No students to show." << endl;
        return;
    }

    cout << fixed << setprecision(2);
    cout << left << setw(4) << "#" << setw(16) << "First name" << setw(16) << "Last name"
         << setw(8) << "Class" << "Average" << endl;
    cout << string(50, '-') << endl;

    for (size_t i = 0; i < students.size(); i++)
    {
        cout << left << setw(4) << i + 1 << setw(16) << students[i].firstName
             << setw(16) << students[i].lastName << setw(8) << students[i].className
             << students[i].average << endl;
    }
}

// Show the list and let the user choose one student by number.
// Returns the position in the vector, or -1 if nothing was chosen.
int pickStudent(const vector<Student>& students, const string& action)
{
    if (students.empty())
    {
        cout << "No students yet." << endl;
        return -1;
    }

    printStudents(students);
    int number = readInt("Enter the number of the student to " + action + ": ");

    if (number < 1 || number > (int)students.size())
    {
        cout << "Invalid number." << endl;
        return -1;
    }
    return number - 1;
}

// ---------- Features ----------

void addStudent(vector<Student>& students)
{
    Student s;
    s.firstName = readText("First name: ");
    s.lastName = readText("Last name: ");
    s.className = readText("Class: ");
    s.average = readAverage("Average grade: ");

    students.push_back(s);
    saveStudents(students);
    cout << "Student added!" << endl;
}

void searchStudents(const vector<Student>& students)
{
    string query = toLower(readText("Search (part of first or last name): "));
    vector<Student> results;

    for (const Student& s : students)
    {
        if (toLower(s.firstName).find(query) != string::npos ||
            toLower(s.lastName).find(query) != string::npos)
            results.push_back(s);
    }

    if (results.empty())
        cout << "No students found." << endl;
    else
        printStudents(results);
}

void updateStudent(vector<Student>& students)
{
    int index = pickStudent(students, "update");
    if (index == -1)
        return;

    students[index].firstName = readText("New first name: ");
    students[index].lastName = readText("New last name: ");
    students[index].className = readText("New class: ");
    students[index].average = readAverage("New average grade: ");

    saveStudents(students);
    cout << "Updated successfully!" << endl;
}

void deleteStudent(vector<Student>& students)
{
    int index = pickStudent(students, "delete");
    if (index == -1)
        return;

    string answer = readLine("Delete " + students[index].firstName + " " +
                             students[index].lastName + "? (y/n): ");

    if (!answer.empty() && tolower((unsigned char)answer[0]) == 'y')
    {
        students.erase(students.begin() + index);
        saveStudents(students);
        cout << "Deleted." << endl;
    }
    else
    {
        cout << "Cancelled." << endl;
    }
}

// The list is passed by value (a copy), so sorting does not change the saved order
void showSorted(vector<Student> students)
{
    sort(students.begin(), students.end(),
         [](const Student& a, const Student& b) { return a.average > b.average; });
    printStudents(students);
}

// ---------- Main program ----------

int main()
{
    vector<Student> students = loadStudents();
    int choice;

    do
    {
        cout << endl;
        cout << "=== Student Manager ===" << endl;
        cout << "1. Add student" << endl;
        cout << "2. Show all students" << endl;
        cout << "3. Search" << endl;
        cout << "4. Update student" << endl;
        cout << "5. Delete student" << endl;
        cout << "6. Show sorted by average (best first)" << endl;
        cout << "7. Exit" << endl;
        choice = readInt("Choice: ");

        switch (choice)
        {
            case 1: addStudent(students); break;
            case 2: printStudents(students); break;
            case 3: searchStudents(students); break;
            case 4: updateStudent(students); break;
            case 5: deleteStudent(students); break;
            case 6: showSorted(students); break;
            case 7: cout << "Goodbye!" << endl; break;
            default: cout << "Invalid choice, try again." << endl;
        }
    }
    while (choice != 7);

    return 0;
}
