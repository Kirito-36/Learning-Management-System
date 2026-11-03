#pragma once
#include <optional>
#include <vector>

#include "domain/Records.hpp"

namespace lms::data {

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;

    virtual int addDepartment(const domain::Department& department) = 0;
    virtual std::optional<domain::Department> findDepartment(int id) const = 0;
    virtual std::vector<domain::Department> listDepartments() const = 0;
    virtual void setHeadOfDepartment(int departmentId, int userId) = 0;

    virtual int addCourse(const domain::Course& course) = 0;
    virtual std::optional<domain::Course> findCourse(int id) const = 0;
    virtual std::vector<domain::Course> listCourses() const = 0;
    virtual std::vector<domain::Course> coursesForLecturer(int lecturerId) const = 0;
    virtual std::vector<domain::Course> coursesForStudent(int studentId) const = 0;

    virtual void assignLecturer(int courseId, int lecturerId) = 0;
    virtual bool isLecturerOfCourse(int courseId, int lecturerId) const = 0;

    virtual int enrol(int studentId, int courseId, domain::Timestamp at) = 0;
    virtual bool isEnrolled(int studentId, int courseId) const = 0;
};

}  // namespace lms::data
