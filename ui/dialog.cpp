#include "ui/dialog.h"
#include "ui_dialog.h"
#include "cpp_dep/cpp_dep.hpp"
#include <boost/graph/depth_first_search.hpp>
#include <QFileInfo>

// -----------------------------------------------------------------------------
//
struct tree_view_builder : public boost::default_dfs_visitor
{
    tree_view_builder(QTreeWidget* tree, std::size_t num_verts)
        : tree_(tree)
    {
        included_.resize(num_verts, false);
    }

    template <typename Vertex, typename Graph>
    void start_vertex(Vertex const& v, Graph const& g)
    {
        std::string const& file = g[v];
        current_ = new QTreeWidgetItem(tree_);
        current_->setText(0, file.c_str());
        current_->setText(1, QStringLiteral("?K"));
        current_->setText(2, QStringLiteral("?K"));
        current_->setCheckState(0, Qt::Checked);
    }

    template <typename Edge, typename Graph>
    void tree_edge(Edge const& e, Graph const& g)
    {
        std::string const& file = g[e.m_target];

        QFileInfo finfo(file.c_str());
        qint64 this_size = finfo.size();
        size_stack_.push_back(this_size);

        current_ = new QTreeWidgetItem(current_);
        current_->setText(0, file.c_str());
        current_->setCheckState(0, Qt::Checked);

   }

    template <typename Vertex, typename Graph>
    void finish_vertex(Vertex const&, Graph const&)
    {
        current_ = current_->parent();
    }

    QTreeWidget* tree_;
    QTreeWidgetItem* current_;
    std::vector<qint64> size_stack_;
    qint64 running_size_;
};


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    cpp_dep::include_graph_t includes = cpp_dep::read_deps_file("C:/Users/Chris/src/local/cpp-size/test/includes-gcc.txt");
    tree_view_builder build_tree(ui->include_hierarchy, boost::num_vertices(includes));
    boost::depth_first_search(includes, boost::visitor(build_tree));
}

Dialog::~Dialog()
{
    delete ui;
}
