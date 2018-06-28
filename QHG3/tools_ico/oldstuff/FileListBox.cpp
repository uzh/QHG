#include "FileListBox.h"

//-----------------------------------------------------------------------------
// constructor
//
FileListBox::FileListBox() {


  add(m_TreeView);

  //Only show the scrollbars when they are necessary:
  set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  
  //Create the Tree model:
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  m_TreeView.set_model(m_refTreeModel);
  
  // allow multiple selections
  m_TreeView.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

  //Add the TreeView's view columns:
  //This number will be shown with the default numeric formatting.
  m_TreeView.append_column("Name", m_Columns.m_col_name);

  // show widget
  show_all_children();
}

//-----------------------------------------------------------------------------
// destructor
//
FileListBox::~FileListBox() {
}

//-----------------------------------------------------------------------------
// addNew
//   show file name only, but keep path hidden
//
void FileListBox::addNew(Glib::ustring sFullPath) {

    size_t pos = sFullPath.find_last_of('/');
    if (pos == std::string::npos) {
        pos = 0;
    } else {
        pos++;
    }
    std::string sFile = sFullPath.substr(pos);
    
    Gtk::TreeModel::Row row = *(m_refTreeModel->append());
    row[m_Columns.m_col_name] = sFile;
    row[m_Columns.m_col_full] = sFullPath;
}

//-----------------------------------------------------------------------------
// delSelected
//
void FileListBox::delSelected() {
    Glib::RefPtr<Gtk::TreeSelection> ts = m_TreeView.get_selection();
    std::vector<Gtk::TreeModel::Path> pathlist;
    pathlist = ts->get_selected_rows();
    std::vector<Gtk::TreeModel::Path>::reverse_iterator it;
    for(it = pathlist.rbegin(); it != pathlist.rend(); ++it) {
       
        Gtk::TreeModel::iterator iter = m_refTreeModel->get_iter(*it);
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring strText = row[m_Columns.m_col_full];
        printf("Erasing entry[%s]\n", strText.c_str());
        m_refTreeModel->erase(row);
    }
}
 
//-----------------------------------------------------------------------------
// clear
//
void FileListBox::clear() {
    m_refTreeModel->clear();
}

//-----------------------------------------------------------------------------
// getNumEntries
//
int FileListBox::getNumEntries() {
    return m_refTreeModel->children().size();
 }

//-----------------------------------------------------------------------------
// getFirstEntry
//
std::string FileListBox::getFirstEntry() {
    m_vFiles.clear();
    m_iCur = 0;

    Gtk::TreeModel::Children cc = m_refTreeModel->children();
    Gtk::TreeIter it;
    for (it = cc.begin(); it != cc.end(); it++) {
        Gtk::TreeModel::Row row = *it;
        Glib::ustring strText = row[m_Columns.m_col_full];
        printf("Pushing [%s]\n", strText.c_str());
        m_vFiles.push_back(strText);
    }
    return getNextEntry();
}

//-----------------------------------------------------------------------------
// getNextEntry_action
//
std::string FileListBox::getNextEntry() {
    std::string sResult = "";
    if (m_iCur < m_vFiles.size()) {
        sResult = m_vFiles[m_iCur++];
    }
    return sResult;
}
