# Student Manager

A console program written in C++ for managing a list of students. Data is saved to a file, so it is still there the next time you open the program.

## Features

- **Add** a student (first name, last name, class, average grade)
- **Show** all students in a table
- **Search** by part of a first or last name (ignores capital letters)
- **Update** a student's details
- **Delete** a student (with confirmation)
- **Sort** by average grade, best first
- **Input checking**: the program does not crash on wrong input, and average grades are checked to be within the valid range
- **Names with spaces** work, for example "Ana Maria"

## Example

```
=== Student Manager ===
1. Add student
2. Show all students
3. Search
4. Update student
5. Delete student
6. Show sorted by average (best first)
7. Exit
Choice: 6
#   First name      Last name       Class   Average
--------------------------------------------------
1   Cvetan          Kolev           10A     5.90
2   Ana Maria       Petrova         10A     5.50
3   Marko           Ivanov          10C     5.00
```

## How to run

### Option 1: Visual Studio (Windows)

1. Create a new **Console App** project (C++).
2. Replace the contents of the main `.cpp` file with the code from `students.cpp`.
3. Press **Ctrl + F5** to run.

### Option 2: Command line (g++)

```
g++ -std=c++11 -o students students.cpp
./students
```

On Windows, run `students.exe` instead of `./students`.

## How the data is stored

Students are saved in `students.csv` (created automatically in the folder where you run the program). Each line is one student, with the fields separated by semicolons:

```
Ana Maria;Petrova;10A;5.5
```

## Settings

At the top of `students.cpp` you can change the lowest and highest allowed average grade (`MIN_GRADE` and `MAX_GRADE`) to match your school's grading scale.

## What I practiced

- Using a `struct` and `vector` to store data
- Reading and writing files with `fstream`
- Splitting text with `stringstream`
- Validating user input
- Splitting a program into small functions

## Ideas for improvement

- A graphical interface (for example with Qt)
- Statistics: class average, best student per class
- Export the list to a file that Excel can open
