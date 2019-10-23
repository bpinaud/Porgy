/**
 *
 * This file is part of Porgy (http://tulip.labri.fr/TulipDrupal/?q=porgy)
 *
 * from LaBRI, University of Bordeaux, Inria and King's College London
 *
 * Porgy is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Porgy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this Porgy.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <portgraph/PorgyConstants.h>

#include <porgy/PortQt.h>

#include "createPort.h"

#include <tulip/GlyphManager.h>
#include <tulip/TlpQtTools.h>
#include <tulip/PluginLister.h>

#include "ui_DialogPort.h"

using namespace std;
using namespace tlp;

CreatePort::CreatePort(QWidget *parent) : QDialog(parent), ui(new Ui::DialogPort) {
  init(tlp::colorToQColor(PorgyConstants::DEFAULT_COLOR));
}

// modif d'un port existant
CreatePort::CreatePort(QWidget *parent, const PortQt &port)
    : QDialog(parent), ui(new Ui::DialogPort) {
  init(port.getColor());
  ui->lineEditName->setText(port.getName());
  ui->comboBoxGlyph->setCurrentIndex(ui->comboBoxGlyph->findText(
      tlpStringToQString(GlyphManager::glyphName(port.getShape()))));
}

CreatePort::~CreatePort() {
  delete ui;
}

void CreatePort::init(const QColor &color) {
  ui->setupUi(this);
  std::list<std::string> glyphs = tlp::PluginLister::availablePlugins<tlp::Glyph>();
  glyphs.remove("2D - Billboard"); // remove Billboard. It causes to much troubles
  glyphs.remove("2D - Square");    // 2D - Square will be inserted at the beginning
                                   // of the list
  ui->comboBoxGlyph->addItem("2D - Square");
  for (auto s : glyphs) {
    ui->comboBoxGlyph->addItem(tlp::tlpStringToQString(s));
  }
  ui->ColorPortButton->setDialogParent(this);
  ui->ColorPortButton->setColor(color);
}

QString CreatePort::getName() const {
  return ui->lineEditName->text();
}

NodeShape::NodeShapes CreatePort::getGlyph() const {
  return static_cast<NodeShape::NodeShapes>(
      GlyphManager::glyphId(QStringToTlpString(ui->comboBoxGlyph->currentText())));
}

QColor CreatePort::getColor() const {
  return ui->ColorPortButton->color();
}
