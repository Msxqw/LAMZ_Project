#ifndef THEMES_H
#define THEMES_H

#define DARK_THEME R"(
/* Основной фон */
QWidget {
    background-color: #12121a;
    color: #e0e6ed;
    font-family: 'Segoe UI', sans-serif;
    font-size: 13px;
}

QMainWindow {
    background-color: #12121a;
}

/* Кнопки */
QPushButton {
    background-color: #667eea;
    color: #ffffff;
    border-radius: 8px;
    padding: 8px 16px;
    border: none;
}
QPushButton:hover {
    background-color: #5a67d8;
}
QPushButton:pressed {
    background-color: #4c57c5;
}

/* Поля ввода */
QLineEdit {
    background-color: #1e1e2e;
    border: 1px solid #3a3a4a;
    border-radius: 6px;
    padding: 6px 8px;
    color: #e0e6ed;
}
QLineEdit:focus {
    border-color: #667eea;
}

/* Метки */
QLabel {
    color: #e0e6ed;
}

/* Лог */
QTextEdit {
    background-color: #1a1a2a;
    color: #c0c8e0;
    border-radius: 6px;
    padding: 6px;
    font-family: 'Consolas', monospace;
}

/* Вкладки */
QTabWidget::pane {
    border: none;
    background-color: #12121a;
}
QTabBar::tab {
    background-color: #1e1e2e;
    color: #a0a8c0;
    padding: 8px 12px;
    margin-right: 2px;
}
QTabBar::tab:selected {
    background-color: #667eea;
    color: #ffffff;
}

/* ТАБЛИЦА — ИСПРАВЛЕНИЕ ЗАГОЛОВКОВ! */
QHeaderView::section {
    background-color: #2a2a3a;
    color: #e0e6ed;
    padding: 4px 8px;
    border: 1px solid #404050;
    font-weight: bold;
}

QHeaderView::section:hover {
    background-color: #3a3a4a;
}

QTableWidget {
    gridline-color: #404050;
    background-color: #1e1e2e;
    alternate-background-color: #23233a;
}

QTableWidget::item {
    background-color: #1e1e2e;
    color: #e0e6ed;
    border: 1px solid #404050;
    padding: 6px;
}

QTableWidget::item:selected {
    background-color: #667eea;
    color: white;
}
)"

#endif // THEMES_H
