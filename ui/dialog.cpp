#include "ui/dialog.h"
#include "ui_dialog.h"
#include "cpp_dep/cpp_dep.hpp"
#include <boost/graph/depth_first_search.hpp>
#include <QFileInfo>

// -----------------------------------------------------------------------------
//
struct tree_view_builder : public boost::default_dfs_visitor
{
    tree_view_builder(QTreeWidget* tree)
        : tree_(tree)
        , current_(nullptr)
    { }

    template <typename Vertex, typename Graph>
    void start_vertex(Vertex const& v, Graph const& g)
    {
        std::string const& file = g[v];
        current_ = new QTreeWidgetItem(tree_);
        current_->setText(0, file.c_str());
        size_stack_.push_back(0);
        //current_->setCheckState(0, Qt::Checked);
    }

    template <typename Edge, typename Graph>
    void tree_edge(Edge const& e, Graph const& g)
    {
        std::string const& file = g[e.m_target];

        QFileInfo finfo(file.c_str());
        qint64 this_size = finfo.size();

        std::transform(
            size_stack_.begin(),
            size_stack_.end(),
            size_stack_.begin(),
            [this_size](qint64 v)
            {
                return v + this_size;
            }
        );

        size_stack_.push_back(this_size);

        current_ = new QTreeWidgetItem(current_);
        current_->setText(0, file.c_str());
        //current_->setCheckState(0, Qt::Checked);
    }

    template <typename Vertex, typename Graph>
    void finish_vertex(Vertex const&, Graph const&)
    {
        std::string s = std::to_string(size_stack_.back());
        current_->setText(1, s.c_str());
        size_stack_.pop_back();
        current_ = current_->parent();
    }

    QTreeWidget* tree_;
    QTreeWidgetItem* current_;
    std::vector<qint64> size_stack_;
};


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}
