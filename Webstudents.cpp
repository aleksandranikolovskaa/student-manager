// Student Manager - web version
// Same data and logic as the console program, but the interface is a web page.
// Data is still saved in students.csv.
//
// Needs httplib.h in the same folder.
// Compile: g++ main.cpp -o app -pthread
// Run:     ./app   (then open port 8080 in the browser)

#include "httplib.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;

const string FILE_NAME = "students.csv";
const double MIN_GRADE = 1.0;   // change these two to match your school's grading scale
const double MAX_GRADE = 6.0;

struct Student
{
    string firstName;
    string lastName;
    string className;
    double average = 0;
};

// The list lives here while the program runs
vector<Student> students;

// ---------- Helpers (same as before) ----------

string toLower(string text)
{
    for (char& c : text)
        c = (char)tolower((unsigned char)c);
    return text;
}

vector<Student> loadStudents()
{
    vector<Student> list;
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
                list.push_back(s);
            }
            catch (...) {}   // skip broken lines
        }
    }
    return list;
}

void saveStudents(const vector<Student>& list)
{
    ofstream file(FILE_NAME);
    for (const Student& s : list)
        file << s.firstName << ";" << s.lastName << ";" << s.className << ";" << s.average << "\n";
}

// ---------- New helpers for the web page ----------

// Names go inside HTML, so special characters must be escaped
string escapeHtml(const string& text)
{
    string out;
    for (char c : text)
    {
        if (c == '&') out += "&amp;";
        else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '"') out += "&quot;";
        else if (c == '\'') out += "&#39;";
        else out += c;
    }
    return out;
}

// Format a grade with 2 decimals, like 5.50
string gradeText(double value)
{
    ostringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

// Turn "3" into 3. Returns -1 if it is not a valid student number.
int parseIndex(const string& text)
{
    try
    {
        size_t used;
        int value = stoi(text, &used);
        if (used == text.size() && value >= 0 && value < (int)students.size())
            return value;
    }
    catch (...) {}
    return -1;
}

// Read the form fields into a Student.
// Returns an error message, or "" if everything is fine.
string readForm(const httplib::Request& req, Student& s)
{
    s.firstName = req.get_param_value("first");
    s.lastName = req.get_param_value("last");
    s.className = req.get_param_value("class");
    string average = req.get_param_value("average");

    for (const string* text : {&s.firstName, &s.lastName, &s.className})
    {
        if (text->empty() || text->find(';') != string::npos)
            return "Fill in every field. The ; character is not allowed.";
    }

    replace(average.begin(), average.end(), ',', '.');   // allow 5,50 as well as 5.50
    try
    {
        size_t used;
        s.average = stod(average, &used);
        if (used == average.size() && s.average >= MIN_GRADE && s.average <= MAX_GRADE)
            return "";
    }
    catch (...) {}

    ostringstream message;
    message << "Average must be a number between " << MIN_GRADE << " and " << MAX_GRADE << ".";
    return message.str();
}

// ---------- Page design ----------

const string STYLE = R"(
:root { --bg:#f2f4f8; --card:#ffffff; --ink:#1c2433; --muted:#667085;
        --line:#dde2ea; --accent:#1f5eff; --danger:#c62828; }
* { box-sizing: border-box; }
body { margin:0; background:var(--bg); color:var(--ink);
       font-family: -apple-system, "Segoe UI", Roboto, sans-serif; font-size:17px; }
main { max-width:760px; margin:0 auto; padding:20px 16px 60px; }
h1 { font-size:28px; margin:8px 0 18px; }
h2 { font-size:18px; margin:0 0 12px; }
section { background:var(--card); border:1px solid var(--line); border-radius:12px;
          padding:16px; margin-bottom:16px; }
form.row { display:flex; flex-wrap:wrap; gap:10px; }
input[type=text] { flex:1 1 140px; padding:12px; font-size:17px;
                   border:1px solid var(--line); border-radius:8px; }
button, a.button { padding:12px 18px; font-size:16px; border:0; border-radius:8px;
                   background:var(--accent); color:#fff; cursor:pointer; text-decoration:none; }
button.plain, a.plain { background:#e8ecf3; color:var(--ink); }
button.danger { background:#fdeaea; color:var(--danger); }
label.check { display:flex; align-items:center; gap:8px; padding:0 6px; color:var(--muted); }
.error { background:#fdeaea; color:var(--danger); padding:12px; border-radius:8px; margin-bottom:12px; }
.student { display:flex; align-items:center; gap:12px; padding:12px 0; border-top:1px solid var(--line); }
.student:first-of-type { border-top:0; }
.info { flex:1; min-width:0; }
.name { font-weight:600; }
.meta { color:var(--muted); font-size:15px; }
.grade { font-size:20px; font-weight:700; min-width:56px; text-align:right; }
.actions { display:flex; gap:8px; }
.actions form { margin:0; }
.actions button, .actions a { padding:8px 12px; font-size:15px; }
.empty { color:var(--muted); padding:8px 0; }
)";

string page(const string& body)
{
    return "<!DOCTYPE html><html><head><meta charset='utf-8'>"
           "<meta name='viewport' content='width=device-width, initial-scale=1'>"
           "<title>Student Manager</title><style>" + STYLE + "</style></head>"
           "<body><main>" + body + "</main></body></html>";
}

string errorBox(const string& error)
{
    if (error.empty())
        return "";
    return "<div class='error'>" + escapeHtml(error) + "</div>";
}

// The main page: add form, search, and the list
string homePage(const httplib::Request& req, const string& error)
{
    string rawQuery = req.get_param_value("q");
    string query = toLower(rawQuery);
    bool sorted = req.get_param_value("sort") == "avg";

    // Find which students match the search (we keep their positions)
    vector<size_t> rows;
    for (size_t i = 0; i < students.size(); i++)
    {
        if (toLower(students[i].firstName).find(query) != string::npos ||
            toLower(students[i].lastName).find(query) != string::npos)
            rows.push_back(i);
    }

    if (sorted)
        sort(rows.begin(), rows.end(),
             [](size_t a, size_t b) { return students[a].average > students[b].average; });

    string html = "<h1>Student Manager</h1>";

    // Add form
    html += "<section><h2>Add a student</h2>" + errorBox(error) +
            "<form class='row' action='/add' method='post'>"
            "<input type='text' name='first' placeholder='First name'>"
            "<input type='text' name='last' placeholder='Last name'>"
            "<input type='text' name='class' placeholder='Class'>"
            "<input type='text' name='average' inputmode='decimal' placeholder='Average grade'>"
            "<button>Add student</button></form></section>";

    // Search and sort
    html += "<section><form class='row' action='/' method='get'>"
            "<input type='text' name='q' placeholder='Search by first or last name' value='" +
            escapeHtml(rawQuery) + "'>"
            "<label class='check'><input type='checkbox' name='sort' value='avg'" +
            string(sorted ? " checked" : "") + "> Best average first</label>"
            "<button class='plain'>Search</button></form></section>";

    // The list
    html += "<section><h2>Students (" + to_string(rows.size()) + ")</h2>";
    if (rows.empty())
        html += "<div class='empty'>No students to show.</div>";

    for (size_t i : rows)
    {
        const Student& s = students[i];
        html += "<div class='student'><div class='info'><div class='name'>" +
                escapeHtml(s.firstName) + " " + escapeHtml(s.lastName) + "</div>"
                "<div class='meta'>Class " + escapeHtml(s.className) + "</div></div>"
                "<div class='grade'>" + gradeText(s.average) + "</div>"
                "<div class='actions'>"
                "<a class='button plain' href='/edit?i=" + to_string(i) + "'>Edit</a>"
                "<form action='/delete' method='post' onsubmit=\"return confirm('Delete this student?')\">"
                "<input type='hidden' name='i' value='" + to_string(i) + "'>"
                "<button class='danger'>Delete</button></form></div></div>";
    }
    html += "</section>";

    return page(html);
}

// The page for changing one student
string editPage(int index, const Student& s, const string& error)
{
    string html = "<h1>Edit student</h1><section>" + errorBox(error) +
        "<form class='row' action='/update' method='post'>"
        "<input type='hidden' name='i' value='" + to_string(index) + "'>"
        "<input type='text' name='first' placeholder='First name' value='" + escapeHtml(s.firstName) + "'>"
        "<input type='text' name='last' placeholder='Last name' value='" + escapeHtml(s.lastName) + "'>"
        "<input type='text' name='class' placeholder='Class' value='" + escapeHtml(s.className) + "'>"
        "<input type='text' name='average' inputmode='decimal' placeholder='Average grade' value='" +
        gradeText(s.average) + "'>"
        "<button>Save changes</button>"
        "<a class='button plain' href='/'>Cancel</a></form></section>";
    return page(html);
}

// ---------- Main program ----------

int main()
{
    students = loadStudents();
    httplib::Server server;

    // Show the main page
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(homePage(req, ""), "text/html; charset=utf-8");
    });

    // Add student
    server.Post("/add", [](const httplib::Request& req, httplib::Response& res) {
        Student s;
        string error = readForm(req, s);
        if (!error.empty())
        {
            res.set_content(homePage(req, error), "text/html; charset=utf-8");
            return;
        }
        students.push_back(s);
        saveStudents(students);
        res.set_redirect("/");
    });

    // Show the edit page
    server.Get("/edit", [](const httplib::Request& req, httplib::Response& res) {
        int index = parseIndex(req.get_param_value("i"));
        if (index == -1)
        {
            res.set_redirect("/");
            return;
        }
        res.set_content(editPage(index, students[index], ""), "text/html; charset=utf-8");
    });

    // Save the changes
    server.Post("/update", [](const httplib::Request& req, httplib::Response& res) {
        int index = parseIndex(req.get_param_value("i"));
        if (index == -1)
        {
            res.set_redirect("/");
            return;
        }
        Student s;
        string error = readForm(req, s);
        if (!error.empty())
        {
            res.set_content(editPage(index, s, error), "text/html; charset=utf-8");
            return;
        }
        students[index] = s;
        saveStudents(students);
        res.set_redirect("/");
    });

    // Delete student
    server.Post("/delete", [](const httplib::Request& req, httplib::Response& res) {
        int index = parseIndex(req.get_param_value("i"));
        if (index != -1)
        {
            students.erase(students.begin() + index);
            saveStudents(students);
        }
        res.set_redirect("/");
    });

    cout << "Running on port 8080. Open it from the Ports tab." << endl;
    server.listen("0.0.0.0", 8080);
    return 0;
}
