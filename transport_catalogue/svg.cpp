
#include "svg.h"

namespace svg {
    //---------------------------Color------------------------
    std::ostream& operator<<(std::ostream& out, Color color) {
        std::visit(OstreamColorPrinter{ out }, color);
        return out;
    }

    void OstreamColorPrinter::operator()(std::monostate) const {
        out << "none";
    }
    void OstreamColorPrinter::operator()(std::string color) const {
        out << color;
    }
    void OstreamColorPrinter::operator()(Rgb color) const {
        out << "rgb(" << int(color.red) << "," << int(color.green)
            << "," << int(color.blue) << ")";
    }
    void OstreamColorPrinter::operator()(Rgba color) const {
        out << "rgba(" << int(color.red) << "," << int(color.green)
            << "," << int(color.blue) << "," << color.opacity << ")";
    }
    //------------------------Object------------------------
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        // Делегируем вывод тега своим подклассам
        RenderObject(context);
        context.out << std::endl;
    }

    //------------------------Circle------------------------
    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }
    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }
    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    //------------------------Polyline------------------------
    Polyline& Polyline::AddPoint(Point point) {
        AllPoint.push_back(point);
        return *this;
    }
    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        for (int i = 0; i < static_cast<int>(AllPoint.size()); i++) {
            //out << point.x << "," << point.y << " ";
            if (i == static_cast<int>(AllPoint.size()) - 1) {
                out << AllPoint[i].x << "," << AllPoint[i].y;
                continue;
            }
            out << AllPoint[i].x << "," << AllPoint[i].y << " ";
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>";
    }

    //------------------------Text-------------------------------
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = std::move(size);
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " x=\"" << position_.x << "\" y=\"" << position_.y;
        out << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y;
        out << "\" font-size=\"" << size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        out << ">";
        if (!data_.empty()) {
            out << data_;
        }
        out << "</text>";
    }

    //----------------------Document------------------
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
        for (auto& obj : objects_) {
            out << "  ";
            obj.get()->Render(out);
        }
        out << "</svg>";
    }
}

svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays; ++i) {
        double angle = 2 * M_PI * (i % num_rays) / num_rays;
        polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
        if (i == num_rays) {
            break;
        }
        angle += M_PI / num_rays;
        polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
    }
    return polyline;
}
namespace shapes {
    void Star::Draw(svg::ObjectContainer& container) const {
        container.Add(CreateStar(centre_, outer_radius_, inner_radius_, num_rays_).SetFillColor("red").SetStrokeColor("black"));
    }

    void Snowman::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Circle().SetCenter({ head_centre_.x,head_centre_.y + 5 * head_radius_ }).SetRadius(2 * head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        container.Add(svg::Circle().SetCenter({ head_centre_.x,head_centre_.y + 2 * head_radius_ }).SetRadius(1.5 * head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        container.Add(svg::Circle().SetCenter(head_centre_).SetRadius(head_radius_).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));

    }

    void Triangle::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }
}

//корректный вывод
std::ostream& operator << (std::ostream& out, svg::StrokeLineCap a)
{
    if (a == svg::StrokeLineCap::BUTT) {
        return out << "butt";
    }
    else if (a == svg::StrokeLineCap::ROUND) {
        return out << "round";
    }
    else if (a == svg::StrokeLineCap::SQUARE) {
        out << "square";
    }
    return out << "";
}

std::ostream& operator << (std::ostream& out, svg::StrokeLineJoin a)
{
    if (a == svg::StrokeLineJoin::ARCS) {
        return out << "arcs";
    }
    else if (a == svg::StrokeLineJoin::BEVEL) {
        return out << "bevel";
    }
    else if (a == svg::StrokeLineJoin::MITER) {
        return out << "miter";
    }
    else if (a == svg::StrokeLineJoin::MITER_CLIP) {
        return  out << "miter-clip";
    }
    else if (a == svg::StrokeLineJoin::ROUND) {
        return out << "round";
    }
    return out << "";
}