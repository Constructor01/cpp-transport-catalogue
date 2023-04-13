#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <optional>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <variant>
using namespace std::literals;
namespace svg {
    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : red(r), green(g), blue(b), opacity(o) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{};
    struct OstreamColorPrinter {
        std::ostream& out;
        void operator()(std::monostate) const;
        void operator()(std::string) const;
        void operator()(Rgb) const;
        void operator()(Rgba) const;
    };
    std::ostream& operator<<(std::ostream& out, Color color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    template <typename Path>
    class PathProps {
    public:
        Path& SetFillColor(Color color) {
            fill_color_ = color;
            return ChangePath();
        }
        Path& SetStrokeColor(Color color) {
            stroke_color_ = color;
            return ChangePath();
        }
        Path& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return ChangePath();
        }
        Path& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return ChangePath();
        }
        Path& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return ChangePath();
        }
    protected:
        std::optional <Color> fill_color_;
        std::optional <Color> stroke_color_;
        std::optional <double> stroke_width_;
        std::optional <StrokeLineCap> line_cap_;
        std::optional <StrokeLineJoin> line_join_;
        void RenderAttrs(std::ostream& out) const {
            if (fill_color_) {
                out << " fill=\"" << *fill_color_ << "\"";
            }

            if (stroke_color_) {
                out << " stroke=\"" << *stroke_color_ << "\"";
            }

            if (stroke_width_) {
                out << " stroke-width=\"" << *stroke_width_ << "\"";
            }

            if (line_cap_) {
                if (*line_cap_ == StrokeLineCap::BUTT) {
                    out << " stroke-linecap=\"" << "butt" << "\"";
                }
                else if (*line_cap_ == StrokeLineCap::ROUND) {
                    out << " stroke-linecap=\"" << "round" << "\"";
                }
                else if (*line_cap_ == StrokeLineCap::SQUARE) {
                    out << " stroke-linecap=\"" << "square" << "\"";
                }
            }

            if (line_join_) {
                if (*line_join_ == StrokeLineJoin::ARCS) {
                    out << " stroke-linejoin=\"" << "arcs" << "\"";
                }
                else if (*line_join_ == StrokeLineJoin::BEVEL) {
                    out << " stroke-linejoin=\"" << "bevel" << "\"";
                }
                else if (*line_join_ == StrokeLineJoin::MITER) {
                    out << " stroke-linejoin=\"" << "miter" << "\"";
                }
                else if (*line_join_ == StrokeLineJoin::MITER_CLIP) {
                    out << " stroke-linejoin=\"" << "miter-clip" << "\"";
                }
                else if (*line_join_ == StrokeLineJoin::ROUND) {
                    out << " stroke-linejoin=\"" << "round" << "\"";
                }
            }
        }

    private:
        Path& ChangePath() {
            return static_cast<Path&>(*this);
        }
    };

    struct Point {
        Point() = default;
        Point(double x, double y) : x(x), y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out) : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0) : out(out), indent_step(indent_step), indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final :public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> AllPoint;
    };

    class Text :public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point position_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        // Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
        template <typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    protected:
        std::deque<std::unique_ptr<Object>> objects_;
    };

    class Document :public ObjectContainer {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            objects_.emplace_back(std::move(obj));
        }

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;
        // Прочие методы и данные, необходимые для реализации класса Document 
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

}  // namespace svg




namespace shapes {
    class Star :public svg::Drawable {
    public:
        Star(svg::Point centre, double outer_radius, double inner_radius, int num_rays) :centre_(centre), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays) {
        }
        void Draw(svg::ObjectContainer& container) const override;
        virtual ~Star() = default;
    private:
        svg::Point centre_;
        double outer_radius_;
        double inner_radius_;
        int num_rays_;
    };

    class Snowman :public svg::Drawable {
    public:
        Snowman(svg::Point head_centre, double head_radius) :head_centre_(head_centre), head_radius_(head_radius) {
        }
        void Draw(svg::ObjectContainer& container) const override;
        virtual ~Snowman() = default;
    private:
        svg::Point head_centre_;
        double head_radius_;
    };

    class Triangle :public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3) : p1_(p1), p2_(p2), p3_(p3) {
        }
        void Draw(svg::ObjectContainer& container) const override;
        virtual ~Triangle() = default;
    private:
        svg::Point p1_, p2_, p3_;
    };

}
