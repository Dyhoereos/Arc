#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "takephoto.h"
#include <QTableView>
#include <QItemDelegate>
#include <QStandardItemModel>
#include "payment.h"

#include <QProgressDialog>

//MyModel* checkoutModel;
Report *checkoutReport, *vacancyReport, *lunchReport, *wakeupReport,
    *bookingReport, *transactionReport;
bool firstTime = true;
QStack<int> backStack;
QStack<int> forwardStack;

QFuture<void> displayFuture ;
QFuture<void> displayPicFuture;
QFuture<void> transacFuture;
//CaseFiles stuff
QVector<QTableWidget*> pcp_tables;
QVector<QString> pcpTypes;
bool loaded = false;
QString idDisplayed;

int transacNum;

QProgressDialog* dialog;

//QSqlQuery resultssss;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->stackedWidget);

    ui->makeBookingButton->hide();
    //mw = this;

    //default signal of stackedWidget
    //detect if the widget is changed
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(initCurrentWidget(int)));
    connect(dbManager, SIGNAL(dailyReportStatsChanged(QList<int>)), this, SLOT(updateDailyReportStats(QList<int>)));
    curClient = 0;
    curBook = 0;
    trans = 0;
    currentshiftid = 1; // should change this. Set to 1 for testing;

    MainWindow::setupReportsScreen();

    //display logged in user and current shift in status bar
    QLabel *lbl_curUser = new QLabel("Logged in as:   ");
    QLabel *lbl_curShift = new QLabel("Shift Number: ");
    statusBar()->addPermanentWidget(lbl_curUser);
    statusBar()->addPermanentWidget(lbl_curShift);

    dialog = new QProgressDialog();
    dialog->close();

    // Connect signals and slots for futureWatcher.
    connect(&futureWatcher, SIGNAL(finished()), dialog, SLOT(reset()));
    connect(dialog, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
    connect(&futureWatcher, SIGNAL(progressRangeChanged(int,int)), dialog, SLOT(setRange(int,int)));
    connect(&futureWatcher, SIGNAL(progressValueChanged(int)), dialog, SLOT(setValue(int)));

    this->showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*==============================================================================
DETECT WIDGET CHANGING SIGNAL
==============================================================================*/
//initialize widget when getinto that widget
void MainWindow::initCurrentWidget(int idx){
    switch(idx){
        case MAINMENU:  //WIDGET 0
            curClientID = "";
            break;
        case CLIENTLOOKUP:  //WIDGET 1
            initClientLookupInfo();
            ui->tabWidget_cl_info->setCurrentIndex(0);
            //initimageview

            //disable buttons that need a clientId
            ui->pushButton_bookRoom->setEnabled(false);
            ui->pushButton_processPaymeent->setEnabled(false);
            ui->pushButton_editClientInfo->setEnabled(false);
            ui->pushButton_CaseFiles->setEnabled(false);

            break;
        case BOOKINGLOOKUP: //WIDGET 2

            ui->startDateEdit->setDate(QDate::currentDate());
            ui->endDateEdit->setDate(QDate::currentDate().addDays(1));
            getProgramCodes();
            bookingSetup();
            clearTable(ui->bookingTable);
            editOverLap = false;
            //initcode
            /*
            qDebug()<<"Client INFO";
            if(curClient != NULL){
                qDebug()<<"ID: " << curClientID << curClient->clientId;
                qDebug()<<"NAME: " << curClient->fullName;
                qDebug()<<"Balance: " << curClient->balance;
            }
            */
            break;
        case BOOKINGPAGE: //WIDGET 3
            //initcode
            break;
        case PAYMENTPAGE: //WIDGET 4
            popManagePayment();

            break;
        case ADMINPAGE: //WIDGET 5
            //initcode
            break;
        case EDITUSERS: //WIDGET 6
            //initcode
            break;
        case EDITPROGRAM: //WIDGET 7
            //initcode
            break;
        case CASEFILE: //WIDGET 8
            initPcp();
            break;
        case EDITBOOKING: //WIDGET 9
            //initcode
            break;
        case CLIENTREGISTER:    //WIDGET 10
            clear_client_register_form();
            defaultRegisterOptions();           //combobox item add
            if(curClientID != NULL || curClientID != "")
                read_curClient_Information(curClientID);
            break;
        case 11:    //WIDGET 11
            ui->dailyReport_tabWidget->setCurrentIndex(DEFAULTTAB);
            ui->shiftReport_tabWidget->setCurrentIndex(DEFAULTTAB);
            MainWindow::updateDailyReportTables(QDate::currentDate());
            MainWindow::getDailyReportStats(QDate::currentDate());
            MainWindow::updateShiftReportTables(QDate::currentDate(), 1);
            break;
        default:
            qDebug()<<"NO information about stackWidget idx : "<<idx;

    }
}

void MainWindow::on_bookButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(CLIENTLOOKUP);
    addHistory(MAINMENU);
    qDebug() << "pushed page " << MAINMENU;
    /*ui->startDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDate(QDate::currentDate().addDays(1));
    if(firstTime){
        firstTime = false;
        getProgramCodes();
        bookingSetup();

    }
    ui->makeBookingButton->hide();
    ui->monthCheck->setChecked(false);
    */
}

void MainWindow::on_clientButton_clicked()
{
     ui->stackedWidget->setCurrentIndex(CLIENTLOOKUP);
     addHistory(MAINMENU);
     qDebug() << "pushed page " << MAINMENU;
}

void MainWindow::on_paymentButton_clicked()
{
     ui->stackedWidget->setCurrentIndex(PAYMENTPAGE);
     addHistory(MAINMENU);
     qDebug() << "pushed page " << MAINMENU;
}

void MainWindow::on_editbookButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(EDITBOOKING);
    addHistory(MAINMENU);
    qDebug() << "pushed page " << MAINMENU;
}

void MainWindow::on_caseButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(CLIENTLOOKUP);
    addHistory(MAINMENU);
    qDebug() << "pushed page " << MAINMENU;

}

void MainWindow::on_adminButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(ADMINPAGE);
    addHistory(MAINMENU);
    qDebug() << "pushed page " << MAINMENU;

}

/*==============================================================================
DEV TESTING BUTTONS (START)
==============================================================================*/
void MainWindow::on_actionDB_Connection_triggered()
{
    //QSqlQuery results= dbManager->selectAll("Test");
    //dbManager->printAll(results);
//    QStringList* data = new QStringList();
//    *data << "11" << "12" << "21" << "22";
//    checkoutModel->setData(data, 2, 2);
}

void MainWindow::on_actionTest_Query_triggered()
{
    //ui->stackedWidget->setCurrentIndex(11);
    qDebug() << "test";
}

void MainWindow::on_actionFile_Upload_triggered()
{
    QString strFilePath = MainWindow::browse();
    if (!strFilePath.isEmpty())
    {
        QtConcurrent::run(dbManager, &DatabaseManager::uploadThread, strFilePath);
    }
    else
    {
        qDebug() << "Empty file path";
    }
}

void MainWindow::on_actionDownload_Latest_Upload_triggered()
{
    QtConcurrent::run(dbManager, &DatabaseManager::downloadThread);
}

void MainWindow::on_actionPrint_Db_Connections_triggered()
{
    dbManager->printDbConnections();
}

void MainWindow::on_actionUpload_Display_Picture_triggered()
{
    QString strFilePath = MainWindow::browse();
    if (!strFilePath.isEmpty())
    {
        QtConcurrent::run(dbManager, &DatabaseManager::uploadProfilePicThread, strFilePath);
    }
    else
    {
        qDebug() << "Empty file path";
    }
}

void MainWindow::on_actionDownload_Profile_Picture_triggered()
{
    QImage* img = new QImage();
    img->scaledToWidth(300);
    dbManager->downloadProfilePic(img);

    MainWindow::addPic(*img);
}
/*==============================================================================
DEV TESTING BUTTONS (END)
==============================================================================*/
/*==============================================================================
DEV TESTING AUXILIARY FUNCTIONS (START)
==============================================================================*/
QString MainWindow::browse()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    QString strFilePath = dialog.getOpenFileName(this, tr("SelectFile"), "", tr("All Files (*.*)"));

    return strFilePath;
}
/*==============================================================================
DEV TESTING AUXILIARY FUNCTIONS (END)
==============================================================================*/
/*==============================================================================
REPORT FUNCTIONS (START)
==============================================================================*/
//void MainWindow::updateCheckoutModel()
//{
//    QSqlDatabase tempDb = QSqlDatabase::database();
//    QString connName = QString::number(dbManager->getDbCounter());
//    if (dbManager->createDatabase(&tempDb, connName))
//    {
//        QSqlQuery query;
//        if (dbManager->getCheckoutQuery(&query, QDate::currentDate()))
//        {
////            int numCols = query.record().count();

////            int numRows = 0;
////            QStringList *data = new QStringList();
////            while (query.next()) {
////                numRows++;
////                for (int i = 0; i < numCols; ++i)
////                {
////                    *data << query.value(i).toString();
////                }
////            }
////            checkoutModel->setData(data, numRows, numCols);
//            qDebug() << "test";
//            //report->setData(&query);
//        }
//        tempDb.close();
//        QSqlDatabase::removeDatabase(connName);
//    }
//}


//void MainWindow::updateCheckoutView(QDate date)
//{
//    QSqlQuery query;

//    ui->checkout_tableWidget->setRowCount(0);
//    if (dbManager->getCheckoutQuery(&query, date))
//    {
//        int rowNo = 0;
//        while(query.next())
//        {
//            ui->checkout_tableWidget->insertRow(rowNo);
//            for (int colNo = 0; colNo < NUM_CHKOUT_TBL_COLS; ++colNo)
//            {
//                if (colNo == 5 || colNo == 6)
//                {
//                    ui->checkout_tableWidget->setItem(rowNo, colNo,
//                        new QTableWidgetItem("tbd"));
//                }
//                else if (colNo == 7)
//                {
//                    ui->checkout_tableWidget->setItem(rowNo, colNo,
//                        new QTableWidgetItem(query.value(5).toString()));
//                }
//                else
//                {
//                    ui->checkout_tableWidget->setItem(rowNo, colNo,
//                        new QTableWidgetItem(query.value(colNo).toString()));
//                }
//            }
//            rowNo++;
//        }
//    }
//    else
//    {
//        qDebug() << "updateCheckoutView() failed";
//    }
//    ui->checkout_tableWidget->show();
//}

//void MainWindow::updateVacancyView(QDate date)
//{
//    QSqlQuery query;
//    ui->vacancy_tableWidget->setRowCount(0);
//    if (dbManager->getVacancyQuery(&query, date))
//    {
//        int rowNo = 0;
//        while(query.next())
//        {
//            ui->vacancy_tableWidget->insertRow(rowNo);
//            for (int colNo = 0; colNo < NUM_VACANCY_TBL_COLS; ++colNo)
//            {
//                ui->vacancy_tableWidget->setItem(rowNo,
//                    colNo, new QTableWidgetItem(query.value(colNo).toString()));
//            }
//            rowNo++;
//        }
//    }
//    else
//    {
//        qDebug() << "updateVacancyView() failed";
//    }
//    ui->vacancy_tableWidget->show();
//}

//void MainWindow::updateLunchView(QDate date)
//{
//    QSqlQuery query;
//    ui->lunch_tableWidget->setRowCount(0);
//    if (dbManager->getLunchQuery(&query, date))
//    {
//        int rowNo = 0;
//        while(query.next())
//        {
//            ui->lunch_tableWidget->insertRow(rowNo);
//            for (int colNo = 0; colNo < NUM_LUNCH_TBL_COLS; ++colNo)
//            {
//                ui->lunch_tableWidget->setItem(rowNo,
//                    colNo, new QTableWidgetItem(query.value(colNo).toString()));
//            }
//            rowNo++;
//        }
//    }
//    else
//    {
//        qDebug() << "updateLunchView() failed";
//    }
//    ui->lunch_tableWidget->show();
//}

//void MainWindow::updateWakeupView(QDate date)
//{
//    QSqlQuery query;
//    ui->wakeup_tableWidget->setRowCount(0);
//    if (dbManager->getWakeupQuery(&query, date))
//    {
//        int rowNo = 0;
//        while(query.next())
//        {
//            ui->wakeup_tableWidget->insertRow(rowNo);
//            for (int colNo = 0; colNo < NUM_WAKEUP_TBL_COLS; ++colNo)
//            {
//                ui->wakeup_tableWidget->setItem(rowNo,
//                    colNo, new QTableWidgetItem(query.value(colNo).toString()));
//            }
//            rowNo++;
//        }
//    }
//    else
//    {
//        qDebug() << "updateWakeupView() failed";
//    }
//    ui->wakeup_tableWidget->show();
//}
/*==============================================================================
REPORT FUNCTIONS (END)
==============================================================================*/

//COLIN STUFF /////////////////////////////////////////////////////////////////

void MainWindow::on_lunchCheck_clicked()
{
   QDate testDate = QDate::currentDate();
   testDate = testDate.addDays(32);
//   QDate otherDate = testDate.addDays(35);
  //curClient = new Client();
   //curClient->clientId = "1";

   MyCalendar* mc = new MyCalendar(this, curBook->startDate,curBook->endDate, curClient,1);
   mc->exec();
}

void MainWindow::on_paymentButton_2_clicked()
{
    trans = new transaction();
    double owed;
    //owed = curBook->cost;
    owed = ui->costInput->text().toDouble();
    QString note = "Booking: " + curBook->stringStart + " to " + curBook->stringEnd + " Cost: " + QString::number(curBook->cost, 'f', 2);
    payment * pay = new payment(this, trans, curClient->balance, owed , curClient, note, true);
    pay->exec();
    ui->stayLabel->setText(QString::number(curClient->balance, 'f', 2));
    qDebug() << "Done";


}
void MainWindow::on_startDateEdit_dateChanged()
{

    if(ui->startDateEdit->date() > ui->endDateEdit->date()){
        QDate newD = ui->startDateEdit->date().addDays(1);
        ui->endDateEdit->setDate(newD);
    }
    clearTable(ui->bookingTable);
    ui->makeBookingButton->hide();
}

void MainWindow::on_wakeupCheck_clicked()
{
    MyCalendar* mc = new MyCalendar(this, curBook->startDate,curBook->endDate, curClient,2);
    mc->exec();
}

void MainWindow::on_endDateEdit_dateChanged()
{
    if(editOverLap){

    }
    else{
        editOverLap = false;
        ui->monthCheck->setChecked(false);
    }
    clearTable(ui->bookingTable);
    ui->makeBookingButton->hide();
}

void MainWindow::on_monthCheck_clicked(bool checked)
{
    clearTable(ui->bookingTable);
    ui->makeBookingButton->hide();
    if(checked)
    {
        editOverLap = false;
        QDate month = ui->startDateEdit->date();
        //month = month.addMonths(1);
        int days = month.daysInMonth();
        days = days - month.day();
        month = month.addDays(days + 1);
        ui->endDateEdit->setDate(month);
        ui->monthCheck->setChecked(true);
    }
    else{
        ui->monthCheck->setChecked(true);
        editOverLap = true;
    }
}
void MainWindow::bookingSetup(){

    ui->bookingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->bookingTable->verticalHeader()->hide();
    ui->bookingTable->horizontalHeader()->setStretchLastSection(true);
    ui->bookingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->bookingTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bookingTable->setHorizontalHeaderLabels(QStringList() << "Room #" << "Location" << "Program" << "Description" << "Cost" << "Monthly");

}
void MainWindow::clearTable(QTableWidget * table){
    table->clear();
    table->setRowCount(0);
}

void MainWindow::on_editButton_clicked()
{
    int row = ui->editLookupTable->selectionModel()->currentIndex().row();
    if(row == - 1){
        return;
    }
    curBook = new Booking();
    popBookFromRow();
    popClientFromId(ui->editLookupTable->item(row, 10)->text());
    ui->stackedWidget->setCurrentIndex(EDITPAGE);
    popEditPage();
    setBookSummary();
}
void MainWindow::popClientFromId(QString id){
    QSqlQuery result;
    curClient = new Client();

    result = dbManager->pullClient(id);
    result.seek(0, false);
    curClient->clientId = id;
    curClient->fName = result.record().value("FirstName").toString();
    curClient->mName = result.record().value("MiddleName").toString();
    curClient->lName = result.record().value("LastName").toString();
    curClient->fullName = curClient->fName + " " +  curClient->mName + " "
            + curClient->lName;
    curClient->balance = result.record().value("Balance").toString().toDouble();


}

void MainWindow::popEditPage(){

    QSqlQuery result;
    result = dbManager->getPrograms();
    QString curProgram;
    QString compProgram;
//    int index = 0;
    ui->editOC->setText(QString::number(curBook->cost,'f',2));
    curProgram = curBook->program;

    ui->editRoomLabel->setText(curBook->room);
    ui->editDate->setDate(curBook->endDate);
    ui->editCost->setText(QString::number(curBook->cost));
    ui->editRoom->setEnabled(true);

}

void MainWindow::popBookFromRow(){
    int row = ui->editLookupTable->selectionModel()->currentIndex().row();
    if(row == - 1){
        return;
    }
    curBook->cost = ui->editLookupTable->item(row,7)->text().toDouble();
    curBook->stringStart = ui->editLookupTable->item(row, 1)->text();
    curBook->startDate = QDate::fromString(curBook->stringStart, "yyyy-MM-dd");
    curBook->stringEnd = ui->editLookupTable->item(row, 2)->text();
    curBook->endDate = QDate::fromString(curBook->stringEnd, "yyyy-MM-dd");
    curBook->stayLength = curBook->endDate.toJulianDay() - curBook->startDate.toJulianDay();
    curBook->lunch = ui->editLookupTable->item(row,8)->text();
    if(ui->editLookupTable->item(row,3)->text() == "YES"){
      curBook->monthly = true;
         }
     else{
         curBook->monthly = false;
     }
    curBook->clientId = ui->editLookupTable->item(row, 10)->text();
    curBook->program = ui->editLookupTable->item(row,6)->text();
    curBook->room = ui->editLookupTable->item(row,4)->text();
    curBook->wakeTime = ui->editLookupTable->item(row,9)->text();
    curBook->cost = ui->editLookupTable->item(row, 7)->text().toDouble();
    curBook->bookID = ui->editLookupTable->item(row, 11)->text();

}
void MainWindow::popManagePayment(){
    QStringList dropItems;
    ui->cbox_payDateRange->clear();
    ui->mpTable->clear();
    ui->mpTable->setRowCount(0);
    ui->btn_payDelete->setText("Delete");

    dropItems << "" << "Today" << "Last 3 Days" << "This Month"
              <<  QDate::longMonthName(QDate::currentDate().month() - 1)
              << QDate::longMonthName(QDate::currentDate().month() - 2)
              << "ALL";
    ui->cbox_payDateRange->addItems(dropItems);
}

void MainWindow::on_cbox_payDateRange_activated(int index)
{
    QString startDate;
    QDate endDate = QDate::currentDate();
    QDate hold = QDate::currentDate();
    ui->btn_payDelete->setText("Delete");
    int days, move;
    switch(index){
    case 0:
        return;
        break;
    case 1:
        startDate = QDate::currentDate().toString(Qt::ISODate);
        break;
    case 2:
        hold = hold.addDays(-3);
        startDate = hold.toString(Qt::ISODate);
        break;
    case 3:
        days = hold.daysInMonth();
        days = days - hold.day();
        endDate = hold.addDays(days);
        move = hold.day() -1;
        hold = hold.addDays(move * -1);
        break;
    case 4:
        hold = hold.addMonths(-1);
        days = hold.daysInMonth();

        move = hold.day() -1;
        days = days - hold.day();
        endDate = hold.addDays(days);
        hold = hold.addDays(move * -1);
        break;
    case 5:
        hold = hold.addMonths(-2);
        days = hold.daysInMonth();

        move = hold.day() -1;
        days = days - hold.day();
        endDate = hold.addDays(days);
        hold = hold.addDays(move * -1);
        break;
    case 6:
        hold = QDate::fromString("1970-01-01", "yyyy-MM-dd");
        endDate = QDate::fromString("2222-01-01", "yyyy-MM-dd");
        break;

    }
    QStringList heads;
    QStringList cols;
    QSqlQuery tempSql = dbManager->getTransactions(hold, endDate);
    heads << "Date"  <<"First" << "Last" << "Amount" << "Type" << "Method" << "Notes"  << "" << "";
    cols << "Date" <<"FirstName"<< "LastName"  << "Amount" << "TransType" << "Type" << "Notes" << "TransacId" << "ClientId";
    populateATable(ui->mpTable, heads, cols, tempSql, false);
    ui->mpTable->setColumnHidden(7, true);
    ui->mpTable->setColumnHidden(8, true);

}


void MainWindow::on_btn_payListAllUsers_clicked()
{
    ui->btn_payDelete->setText("Add Payment");
    QStringList cols;
    QStringList heads;
    QSqlQuery tempSql = dbManager->getOwingClients();
    heads << "First" << "Last" << "DOB" << "Balance" << "";
    cols << "FirstName" << "LastName" << "Dob" << "Balance" << "ClientId";
    ui->mpTable->setColumnHidden(4, true);
    populateATable(ui->mpTable, heads, cols, tempSql, false);

}

void MainWindow::on_editSearch_clicked()
{
    ui->editLookupTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->editLookupTable->verticalHeader()->hide();
    ui->editLookupTable->horizontalHeader()->setStretchLastSection(true);
    ui->editLookupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->editLookupTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->editLookupTable->setRowCount(0);
    ui->editLookupTable->clear();
    ui->editLookupTable->setHorizontalHeaderLabels(QStringList()
                                                << "Created" << "Start" << "End" << "Monthly" << "Room" << "Client" << "Program" << "Cost"
                                                 << "Lunch" << "Wakeup" << "" << "");
    ui->editLookupTable->setColumnHidden(10, true);
    ui->editLookupTable->setColumnHidden(11, true);

    QSqlQuery result;
    QString user = "";
    if(ui->editClient->text() != ""){
        user = ui->editClient->text();
    }
    result = dbManager->getActiveBooking(user, true);
//    int numCols = result.record().count();
    //dbManager->printAll(result);
    int x = 0;
    int qt = result.size();
    qDebug() << qt;



    while (result.next()) {
        ui->editLookupTable->insertRow(x);


        QStringList row;
        row << result.value(1).toString() << result.value(7).toString() << result.value(8).toString() << result.value(12).toString()
            << result.value(3).toString() << result.value(13).toString() << result.value(5).toString() << result.value(6).toString()
                  << result.value(9).toString() << result.value(10).toString() << result.value(4).toString() << result.value(0).toString();
        for (int i = 0; i < 12; ++i)
        {
            ui->editLookupTable->setItem(x,i, new QTableWidgetItem(row.at(i)));


        }
        x++;


    }
}
void MainWindow::on_bookingSearchButton_clicked()
{
    if(!book.checkValidDate(ui->startDateEdit->date(), ui->endDateEdit->date())){
        //Pop up error or something
        return;
    }
    ui->bookingTable->setRowCount(0);
    ui->bookingTable->clear();
    ui->bookingTable->setHorizontalHeaderLabels(QStringList() << "Room #" << "Location" << "Program" << "Description" << "Cost" << "Monthly");
    QString program = ui->programDropdown->currentText();
    QSqlQuery result = dbManager->getCurrentBooking(ui->startDateEdit->date(), ui->endDateEdit->date(), program);
    int numCols = result.record().count();

    int x = 0;
    while (result.next()) {
        ui->bookingTable->insertRow(x);
        for (int i = 0; i < numCols; ++i)
        {
            if(i == 4){
                ui->bookingTable->setItem(x,i, new QTableWidgetItem(QString::number(result.value(i).toString().toDouble(), 'f', 2)));
                continue;
            }
            ui->bookingTable->setItem(x,i, new QTableWidgetItem(result.value(i).toString()));


        }

        x++;

    }

    ui->makeBookingButton->show();
}
//PARAMS - The table, list of headers, list of table column names, the sqlquery result, STRETCH - stretch mode true/false
void MainWindow::populateATable(QTableWidget * table, QStringList headers, QStringList items, QSqlQuery result, bool stretch){
    table->clear();
    table->setRowCount(0);

    if(headers.length() != items.length())
        return;

    if(stretch)
        table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->hide();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    int colCount = headers.size();
    table->setColumnCount(colCount);
    if(headers.length() != 0){
        table->setHorizontalHeaderLabels(headers);
    }
    int x = 0;
    while(result.next()){
        table->insertRow(x);
        for(int i = 0; i < colCount; i++){
            table->setItem(x, i, new QTableWidgetItem(result.value(items.at(i)).toString()));


        }


        x++;
    }
}

void MainWindow::setBooking(int row){
    curBook->clientId = curClient->clientId;
    curBook->startDate = ui->startDateEdit->date();
    curBook->endDate = ui->endDateEdit->date();
    curBook->stringStart = ui->startDateEdit->date().toString(Qt::ISODate);
    curBook->stringEnd = ui->endDateEdit->date().toString(Qt::ISODate);
    curBook->monthly = ui->monthCheck->isChecked();
    curBook->program = ui->programDropdown->currentText();
    curBook->room = ui->bookingTable->item(row,0)->text();
    curBook->stayLength = ui->endDateEdit->date().toJulianDay() - ui->startDateEdit->date().toJulianDay();
    double potentialCost = 999999;
    double dailyCost = 0;
    QString dayCost = QString::number(ui->bookingTable->item(row, 4)->text().toDouble(), 'f', 2);
    dailyCost = dayCost.toDouble();
    dailyCost = curBook->stayLength * dailyCost;
    if(ui->monthCheck->isChecked()){

        potentialCost = ui->bookingTable->item(row, 5)->text().toInt();
        if(dailyCost < potentialCost){
            curBook->cost = dailyCost;
        }
        else{
            curBook->cost = potentialCost;
        }
    }
    else{
        curBook->cost = dailyCost;
    }

}

void MainWindow::on_makeBookingButton_clicked()
{
    addHistory(BOOKINGLOOKUP);
    int row = ui->bookingTable->selectionModel()->currentIndex().row();
    if(row == - 1){
        return;
    }
    //curClient = new Client();
   //popClientFromId("1");
    ui->stackedWidget->setCurrentIndex(BOOKINGPAGE);
//    int rowNum = ui->bookingTable->columnCount();
    QStringList data;
    curBook = new Booking;
    setBooking(row);
    ui->stackedWidget->setCurrentIndex(BOOKINGPAGE);
    populateBooking();
    ui->makeBookingButton_2->setEnabled(true);

}
void MainWindow::populateBooking(){
    //room, location, program, description, cost, program, start, end, stayLength
    ui->startLabel->setText(curBook->stringStart);
    ui->endLabel->setText(curBook->stringEnd);
    ui->roomLabel->setText(curBook->room);
    ui->costInput->setText(QString::number(curBook->cost));
    ui->programLabel->setText(curBook->program);
    ui->lengthOfStayLabel->setText(QString::number(curBook->stayLength));
    // - curBook->cost + curBook->paidTotal, 'f', 2)
    ui->stayLabel->setText(QString::number(curClient->balance));
    if(curBook->monthly){
        ui->monthLabel->setText("YES");
    }
    else{
        ui->monthLabel->setText("NO");
    }
}

void MainWindow::getProgramCodes(){
    QSqlQuery result = dbManager->getPrograms();
//    int i = 0;
    ui->programDropdown->clear();
    while(result.next()){
        ui->programDropdown->addItem(result.value(0).toString());
    }
}
void MainWindow::setBookSummary(){
    ui->editStartDate->setText(curBook->stringStart);
    ui->editEndDate->setText(curBook->stringEnd);
    ui->editCostLabel->setText(QString::number(curBook->cost, 'f', 2));
    curBook->monthly == true ? ui->editMonthly->setText("Yes") : ui->editMonthly->setText("No");
    ui->editProgramLabel->setText(curBook->program);
    ui->editLengthOfStay->setText(QString::number(curBook->stayLength));
    ui->editRoomLabel_2->setText(curBook->room);
    ui->editCost->setText(QString::number(curBook->cost, 'f', 2));
    //ui->editRefundAmt->setText(QString::number(ui->editOC->text().toDouble() - curBook->cost));

}

void MainWindow::on_editUpdate_clicked()
{
    double updateBalance;
    if(!checkNumber(ui->editCost->text()))
        return;

    ui->editRoom->setEnabled(true);
    ui->editDate->setEnabled(true);
    if(ui->editRefundLabel->text() == "Refund"){
        curBook->monthly = false;
        updateBalance = curClient->balance + ui->editRefundAmt->text().toDouble();
    }
    else{
        updateBalance = curClient->balance - ui->editRefundAmt->text().toDouble();
    }
    curBook->cost = ui->editCost->text().toDouble();
    if(!dbManager->updateBalance(updateBalance, curClient->clientId)){
        qDebug() << "Error inserting new balance";
        return;
    }
    ui->editOC->setText(QString::number(curBook->cost, 'f', 2));
//    double qt = ui->editRefundAmt->text().toDouble();
    ui->editRefundAmt->setText("0.0");

    curClient->balance = updateBalance;
    curBook->endDate = ui->editDate->date();
    curBook->stringEnd = curBook->endDate.toString(Qt::ISODate);

    updateBooking(*curBook);
    setBookSummary();
    dbManager->removeLunchesMulti(curBook->endDate, curClient->clientId);
    dbManager->deleteWakeupsMulti(curBook->endDate, curClient->clientId);
    curBook->stayLength = curBook->endDate.toJulianDay() - curBook->startDate.toJulianDay();
}

double MainWindow::calcRefund(QDate old, QDate n){
    int updatedDays = n.toJulianDay() - old.toJulianDay();
    double cpd;
    double updatedCost;
    if(updatedDays > 0){
        //DO A POPUP ERROR HERE
        if(curBook->monthly){
            qDebug() << "NOT POSSIBLE -> is a monthly booking";
            ui->editDate->setDate(curBook->endDate);
            updatedDays = 0;
        }
        cpd = curBook->cost / (double)curBook->stayLength;
        //curBook->cost += updatedDays * cpd;
        return updatedDays * cpd;
    }
    else{
        //ERROR POPUP FOR INVALID DATe
        if(n < curBook->startDate){


            ui->editDate->setDate(curBook->startDate);
            updatedDays = curBook->stayLength * -1;

        }
        cpd = curBook->cost / (double)curBook->stayLength;
        updatedCost = cpd * (curBook->stayLength + updatedDays);

        if(curBook->monthly){
            double normalRate = dbManager->getRoomCost(curBook->room);
            updatedCost = normalRate * (curBook->stayLength + updatedDays);
            if(updatedCost > curBook->cost)
                return 0;

        }
        return (curBook->cost - updatedCost) * -1;
    }
}

bool MainWindow::checkNumber(QString num){
    int l = num.length();
    int period = 0;
    char copy[l];
    strcpy(copy, num.toStdString().c_str());
    for(int i = 0; i < num.length(); i++){
        if(copy[i] == '.'){
            if(period)
                return false;
            period++;
            continue;
        }

        if(!isdigit(copy[i]))
            return false;
    }
    return true;
}
bool MainWindow::updateBooking(Booking b){
    QString query;
    QString monthly;
    curBook->monthly == true ? monthly = "YES" : monthly = "NO";
    query = "UPDATE BOOKING SET " +
            QString("SpaceID = '") + b.room + "', " +
            QString("ProgramCode = '") + b.program + "', " +
            QString("Cost = '") + QString::number(b.cost) + + "', " +
            QString("EndDate = '") + b.stringEnd + "', " +
            QString("Lunch = '") + b.lunch + "', " +
            QString("Wakeup = '") + b.wakeTime + "', " +
            QString("Monthly = '") + monthly + "'" +
            QString(" WHERE BookingId = '") +
            b.bookID + "'";
    return dbManager->updateBooking(query);
}
void MainWindow::on_btn_payDelete_clicked()
{
    if(ui->btn_payDelete->text() == "Cash Cheque")
    {
        int index = ui->mpTable->selectionModel()->currentIndex().row();
        if(index == -1)
            return;
        updateCheque(index);
    }
    else if(ui->btn_payDelete->text() == "Delete"){
        int index = ui->mpTable->selectionModel()->currentIndex().row();
        if(index == -1)
            return;

        getTransactionFromRow(index);
    }
    else{
        int index = ui->mpTable->selectionModel()->currentIndex().row();
        if(index == -1)
            return;
        handleNewPayment(index);
    }
}

void MainWindow::handleNewPayment(int row){
    curClient = new Client();
    trans = new transaction();
    curClient->clientId = ui->mpTable->item(row,4)->text();
    double balance = ui->mpTable->item(row, 3)->text().toDouble();
    curClient->balance = balance;
    QString note = "Paying Outstanding Balance";

    payment * pay = new payment(this, trans, curClient->balance, 0 , curClient, note, true);
    pay->exec();
    ui->mpTable->removeRow(row);
}

void MainWindow::updateCheque(int row){
    QString transId = ui->mpTable->item(row, 6)->text();
    double retAmt = ui->mpTable->item(row, 3)->text().toDouble();
    QString clientId = ui->mpTable->item(row, 5)->text();
    curClient = new Client();
    popClientFromId(clientId);
    double curBal = curClient->balance + retAmt;
    if(dbManager->setPaid(transId)){
        if(!dbManager->updateBalance(curBal, clientId)){
                qDebug() << "BIG ERROR - removed transacton but not update balance";
                return;
        }
    }
    ui->mpTable->removeRow(row);
}

void MainWindow::getTransactionFromRow(int row){
    QString transId = ui->mpTable->item(row, 7)->text();

    QString type = ui->mpTable->item(row, 4)->text();
    double retAmt = ui->mpTable->item(row, 3)->text().toDouble();
    QString clientId = ui->mpTable->item(row, 8)->text();
    curClient = new Client();
    popClientFromId(clientId);
    double curBal = curClient->balance;

    if(type == "Payment"){
        curBal -= retAmt;
    }
    else if(type == "Refund"){
        curBal += retAmt;
    }
    else{
        //error - not a payment or refund
        return;
    }
    dbManager->updateBalance(curBal, clientId);
    dbManager->removeTransaction(transId);
    ui->mpTable->removeRow(row);

}

void MainWindow::on_btn_payOutstanding_clicked()
{
    ui->btn_payDelete->setText("Cash Cheque");
    QSqlQuery result;
    result = dbManager->getOutstanding();
    QStringList headers;
    QStringList cols;
    headers << "Date" << "First" << "Last" << "Amount" << "Notes" << "" << "";
    cols << "Date" << "FirstName" << "LastName" << "Amount" << "Notes" << "ClientId" << "TransacId";
    populateATable(ui->mpTable, headers, cols, result, false);
    ui->mpTable->setColumnHidden(6, true);
    ui->mpTable->setColumnHidden(5, true);
}


void MainWindow::on_editDate_dateChanged(const QDate &date)
{
    ui->editRoom->setEnabled(false);
    if(date > curBook->endDate){
        ui->editDate->setDate(curBook->endDate);
        return;
    }
    if(date < curBook->startDate){
        ui->editDate->setDate(curBook->startDate);
    }

    qDebug() << "Edit date called";
    double refund = 0;
    double newCost;
    QString cost = ui->editCost->text();
    if(!checkNumber(cost)){
        qDebug() << "NON NUMBER";
        return;
    }
    //curBook->cost = QString::number(curBook->cost, 'f', 2).toDouble();

    refund = calcRefund(curBook->endDate, date);
    qDebug() << "REFUNDING" << refund;


    newCost = curBook->cost + refund;
    ui->editCost->setText(QString::number(newCost));



}

void MainWindow::on_editManagePayment_clicked()
{
    trans = new transaction();
    double owed;

    owed = ui->editRefundAmt->text().toDouble();
    bool type;
    ui->editRefundLabel->text() == "Refund" ? type = false : type = true;
    if(!type){
        owed *= -1;
    }
    QString note = "";
    payment * pay = new payment(this, trans, curClient->balance, owed , curClient, note, type);
    pay->exec();
}

void MainWindow::on_editCost_textChanged()
{
    double newCost = ui->editCost->text().toDouble();
    double refund = ui->editCancel->text().toDouble();
    double origCost = ui->editOC->text().toDouble();

    qDebug() << "Original Cost " << ui->editOC->text() << " " <<  origCost;
    if(newCost < origCost){
        ui->editRefundLabel->setText("Refund");
        double realRefund = newCost - origCost + refund;

        if(realRefund > 0)
            realRefund = 0;
        realRefund = realRefund * -1;
        ui->editRefundAmt->setText(QString::number(realRefund, 'f', 2));
    }
    else{
        ui->editRefundLabel->setText("Owed");
        ui->editRefundAmt->setText(QString::number(newCost - origCost, 'f', 2));
    }
}

void MainWindow::on_editCancel_textChanged()
{
    double newCost = ui->editCost->text().toDouble();
    double refund = ui->editCancel->text().toDouble();
    double origCost = ui->editOC->text().toDouble();
    if(newCost < origCost){
        ui->editRefundLabel->setText("Refund");
        double realRefund = newCost - origCost + refund;
        if(realRefund > 0)
            realRefund = 0;
        realRefund = realRefund * -1;
        ui->editRefundAmt->setText(QString::number(realRefund, 'f', 2));
    }
    else{
        ui->editRefundLabel->setText("Owed");
        ui->editRefundAmt->setText(QString::number(newCost - origCost, 'f', 2));
    }
}

void MainWindow::on_editRoom_clicked()
{
    ui->editDate->setEnabled(false);
    EditRooms * edit = new EditRooms(this, curBook);
    edit->exec();
    setBookSummary();
}

void MainWindow::on_pushButton_bookRoom_clicked()
{
    addHistory(CLIENTLOOKUP);
    curClient = new Client();
    int nRow = ui->tableWidget_search_client->currentRow();
    if (nRow <0)
        return;

    curClientID = curClient->clientId = ui->tableWidget_search_client->item(nRow, 0)->text();
    curClient->fName =  ui->tableWidget_search_client->item(nRow, 1)->text();
    curClient->mName =  ui->tableWidget_search_client->item(nRow, 2)->text();
    curClient->lName =  ui->tableWidget_search_client->item(nRow, 3)->text();
    curClient->balance =  ui->tableWidget_search_client->item(nRow, 4)->text().toFloat();

    curClient->fullName = QString(curClient->fName + " " + curClient->mName + " " + curClient->lName);

/*
    qDebug()<<"ID: " << curClientID << curClient->clientId;
    qDebug()<<"NAME: " << curClient->fullName;
    qDebug()<<"Balance: " << curClient->balance;
*/
    ui->stackedWidget->setCurrentIndex(BOOKINGLOOKUP);

}

void MainWindow::on_makeBookingButton_2_clicked()
{
    backStack.clear();
    ui->actionBack->setEnabled(false);
    ui->makeBookingButton_2->setEnabled(false);

    curBook->lunch = "NULL";
    curBook->wakeTime = "NULL";
    QString month;
    if(curBook->monthly){
        month = "YES";
    }
    else{
        month = "NO";
    }
    double cost = QString::number(ui->costInput->text().toDouble(), 'f', 2).toDouble();
    QDate today = QDate::currentDate();
    QString values;
    QString todayDate = today.toString(Qt::ISODate);
    values = "'" + today.toString(Qt::ISODate) + "','" + curBook->stringStart + "','" + curBook->room + "','" +
             curClient->clientId + "','" + curBook->program + "','" + QString::number(cost) + "','" + curBook->stringStart
             + "','" + curBook->stringEnd + "','" + curBook->lunch + "','" + curBook->wakeTime + "'," + "'YES'" + ",'" + month + "','" + curClient->fullName +"'";
//    QDate next = curBook->startDate;
    //QDate::fromString(ui->startLabel->text(), "yyyy-MM-dd");
    curBook->cost = cost;
    if(!dbManager->insertBookingTable(values)){
        qDebug() << "ERROR INSERTING BOOKING";
    }
    if(!dbManager->updateBalance(curClient->balance - curBook->cost, curClient->clientId)){
        qDebug() << "ERROR ADDING TO BALANCE UPDATE";
    }
    /*for(int i = 1; i < curBook->stayLength; i++){
        QDate n = next.addDays(i);
        values = "'" + today.toString(Qt::ISODate) + "','" + n.toString(Qt::ISODate) + "','" + curBook->room + "','" +
                 curBook->clientId + "','" + curBook->program + "','" + QString::number(cost) + "','" + curBook->stringStart
                 + "','" + curBook->stringEnd + "','" + curBook->lunch + "','" + curBook->wakeTime + "'," + "'NO'" + ",'" + month + "','" + "Eunwon'";
        if(!dbManager->insertBookingTable(values)){
            qDebug() << "ERROR INSERTING BOOKING";
        }
    }*/

    ui->stackedWidget->setCurrentIndex(CONFIRMBOOKING);
    populateConfirm();

 }

void MainWindow::populateConfirm(){
    ui->confirmCost->setText(QString::number(curBook->cost, 'f', 2));
    ui->confirmEnd->setText(curBook->stringEnd);
    ui->confirmStart->setText(curBook->stringStart);
    ui->confirmLength->setText(QString::number(curBook->stayLength));
    if(curBook->monthly){
        ui->confirmMonthly->setText("YES");
    }else{
        ui->confirmMonthly->setText("NO");
    }
    ui->confirmPaid->setText(QString::number(curClient->balance));
    ui->confirmProgram->setText(curBook->program);


}

//void MainWindow::on_monthCheck_stateChanged(int arg1)
//{

//}


//END COLIN ////////////////////////////////////////////////////////////////////



void MainWindow::on_EditUserButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(EDITUSERS);
    addHistory(ADMINPAGE);
    qDebug() << "pushed page " << ADMINPAGE;

}

void MainWindow::on_EditProgramButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(EDITPROGRAM);
    addHistory(ADMINPAGE);
    qDebug() << "pushed page " << ADMINPAGE;

}

void MainWindow::on_actionMain_Menu_triggered()
{
    addHistory(ui->stackedWidget->currentIndex());
    ui->stackedWidget->setCurrentIndex(MAINMENU);
}


void MainWindow::on_pushButton_RegisterClient_clicked()
{
    addHistory(CLIENTLOOKUP);
    curClientID = "";
    ui->stackedWidget->setCurrentIndex(CLIENTREGISTER);
    ui->label_cl_infoedit_title->setText("Register Client");
    ui->button_register_client->setText("Register");
    ui->dateEdit_cl_rulesign->setDate(QDate::currentDate());
    defaultRegisterOptions();
}

void MainWindow::on_pushButton_editClientInfo_clicked()
{
    addHistory(CLIENTLOOKUP);
    ui->stackedWidget->setCurrentIndex(CLIENTREGISTER);
    ui->label_cl_infoedit_title->setText("Edit Client Information");
    ui->button_register_client->setText("Edit");
    int nRow = ui->tableWidget_search_client->currentRow();
    if (nRow <0)
        return;
    curClientID = ui->tableWidget_search_client->item(nRow, 0)->text();
}
void MainWindow::on_button_cancel_client_register_clicked()
{
    clear_client_register_form();
    ui->stackedWidget->setCurrentIndex(MAINMENU);
}

void MainWindow::on_reportsButton_clicked()
{
//    MainWindow::updateCheckoutView();
//    MainWindow::updateVacancyView();
//    MainWindow::updateLunchView();
//    MainWindow::updateWakeupView();
    ui->stackedWidget->setCurrentIndex(11);
    addHistory(MAINMENU);
    qDebug() << "pushed page " << MAINMENU;
//    dialog.exec();
}

/*===================================================================
  REGISTRATION PAGE
  ===================================================================*/

//Client Regiter widget [TAKE A PICTURE] button
void MainWindow::on_button_cl_takePic_clicked()
{
    TakePhoto *camDialog = new TakePhoto();

    connect(camDialog, SIGNAL(showPic(QImage)), this, SLOT(addPic(QImage)));
    camDialog->show();
}
/*------------------------------------------------------------------
  add picture into graphicview (after taking picture in pic dialog
  ------------------------------------------------------------------*/
void MainWindow::addPic(QImage pict){

  //  qDebug()<<"ADDPIC";
    profilePic = pict.copy();
    QPixmap item = QPixmap::fromImage(pict);
    QPixmap scaled = QPixmap(item.scaledToWidth((int)(ui->graphicsView_cl_pic->width()*0.9), Qt::SmoothTransformation));
    QGraphicsScene *scene = new QGraphicsScene();
    scene->addPixmap(QPixmap(scaled));
    ui->graphicsView_cl_pic->setScene(scene);
    ui->graphicsView_cl_pic->show();
}

void MainWindow::on_button_cl_delPic_clicked()
{
    QGraphicsScene *scene = new QGraphicsScene();
    scene->clear();
    profilePic = (QImage)NULL;
    ui->graphicsView_cl_pic->setScene(scene);

    //delete picture function to database

}

void MainWindow::on_button_clear_client_regForm_clicked()
{
    clear_client_register_form();
}

void MainWindow::getListRegisterFields(QStringList* fieldList)
{
    QString caseWorkerId = QString::number(caseWorkerList.value(ui->comboBox_cl_caseWorker->currentText()));
    if(caseWorkerId == "0")
        caseWorkerId = "";
    *fieldList << ui->lineEdit_cl_fName->text()
               << ui->lineEdit_cl_mName->text()
               << ui->lineEdit_cl_lName->text()
               << ui->dateEdit_cl_dob->date().toString("yyyy-MM-dd")
               << ui->lineEdit_cl_SIN->text()
               << ui->lineEdit_cl_GANum->text()
               << caseWorkerId   //QString::number(caseWorkerList.value(ui->comboBox_cl_caseWorker->currentText())) //grab value from case worker dropdown I don't know how to do it
               << ui->dateEdit_cl_rulesign->date().toString("yyyy-MM-dd")
               << ui->lineEdit_cl_nok_name->text()
               << ui->lineEdit_cl_nok_relationship->text()
               << ui->lineEdit_cl_nok_loc->text()
               << ui->lineEdit_cl_nok_ContactNo->text()
               << ui->lineEdit_cl_phys_name->text()
               << ui->lineEdit_cl_phys_ContactNo->text()
               << ui->lineEdit_cl_supporter_Name->text()
               << ui->lineEdit_cl_supporter_ContactNo->text()
               << ui->lineEdit_cl_supporter2_Name->text()
               << ui->lineEdit_cl_supporter2_ContactNo->text()
               << ui->comboBox_cl_status->currentText() //grab value from status dropdown
               << ui->plainTextEdit_cl_comments->toPlainText();

}

void MainWindow::clear_client_register_form(){
    ui->lineEdit_cl_fName->clear();
    ui->lineEdit_cl_mName->clear();
    ui->lineEdit_cl_lName->clear();
    ui->lineEdit_cl_SIN->clear();
    ui->lineEdit_cl_GANum->clear();
    ui->comboBox_cl_caseWorker->setCurrentIndex(0);
    ui->lineEdit_cl_nok_name->clear();
    ui->lineEdit_cl_nok_relationship->clear();
    ui->lineEdit_cl_nok_loc->clear();
    ui->lineEdit_cl_nok_ContactNo->clear();
    ui->lineEdit_cl_phys_name->clear();
    ui->lineEdit_cl_phys_ContactNo->clear();
    ui->lineEdit_cl_supporter_Name->clear();
    ui->lineEdit_cl_supporter_ContactNo->clear();
    ui->lineEdit_cl_supporter2_Name->clear();
    ui->lineEdit_cl_supporter2_ContactNo->clear();

    ui->comboBox_cl_status->setCurrentIndex(0);
    ui->plainTextEdit_cl_comments->clear();
    QDate defaultDob= QDate::fromString("1990-01-01","yyyy-MM-dd");
    ui->dateEdit_cl_dob->setDate(defaultDob);
    ui->dateEdit_cl_rulesign->setDate(QDate::currentDate());
    on_button_cl_delPic_clicked();
}

//read client information to edit
void MainWindow::read_curClient_Information(QString ClientId){
    QString searchClientQ = "SELECT * FROM Client WHERE ClientId = "+ ClientId;
//    qDebug()<<"SEARCH QUERY: " + searchClientQ;
    QSqlQuery clientInfo = dbManager->execQuery("SELECT * FROM Client WHERE ClientId = "+ ClientId);
//    dbManager->printAll(clientInfo);
    clientInfo.next();

    //input currentValue;

    qDebug()<<"FNAme: "<<clientInfo.value(1).toString()<<"MNAme: "<<clientInfo.value(2).toString()<<"LNAME: "<<clientInfo.value(3).toString();
    qDebug()<<"DOB: "<<clientInfo.value(4).toString() <<"GANUM: "<<clientInfo.value(6).toString()<<"SIN: "<<clientInfo.value(7).toString();

    ui->lineEdit_cl_fName->setText(clientInfo.value(1).toString());

    ui->lineEdit_cl_mName->setText(clientInfo.value(2).toString());
    ui->lineEdit_cl_lName->setText(clientInfo.value(3).toString());
    ui->dateEdit_cl_dob->setDate(QDate::fromString(clientInfo.value(4).toString(),"yyyy-MM-dd"));
    //balnace?
    QString caseWorkerName = caseWorkerList.key(clientInfo.value(21).toInt());
    ui->comboBox_cl_caseWorker->setCurrentText(caseWorkerName);
    ui->lineEdit_cl_SIN->setText(clientInfo.value(6).toString());
    ui->lineEdit_cl_GANum->setText(clientInfo.value(7).toString());
    ui->dateEdit_cl_rulesign->setDate(QDate::fromString(clientInfo.value(8).toString(),"yyyy-MM-dd"));

    //NEXT OF KIN FIELD
    ui->lineEdit_cl_nok_name->setText(clientInfo.value(9).toString());
    ui->lineEdit_cl_nok_relationship->setText(clientInfo.value(10).toString());
    ui->lineEdit_cl_nok_loc->setText(clientInfo.value(11).toString());
    ui->lineEdit_cl_nok_ContactNo->setText(clientInfo.value(12).toString());

    //Physician
    ui->lineEdit_cl_phys_name->setText(clientInfo.value(13).toString());
    ui->lineEdit_cl_phys_ContactNo->setText(clientInfo.value(14).toString());

    //Supporter
    ui->lineEdit_cl_supporter_Name->setText(clientInfo.value(15).toString());
    ui->lineEdit_cl_supporter_ContactNo->setText(clientInfo.value(16).toString());
    ui->lineEdit_cl_supporter2_Name->setText(clientInfo.value(22).toString());
    ui->lineEdit_cl_supporter2_ContactNo->setText(clientInfo.value(23).toString());

    ui->comboBox_cl_status->setCurrentText(clientInfo.value(17).toString());


    QByteArray data = clientInfo.value(20).toByteArray();
    QImage profile = QImage::fromData(data, "PNG");
    addPic(profile);

}

//Client information input and register click
void MainWindow::on_button_register_client_clicked()
{

    if (MainWindow::check_client_register_form())
    {
        QStringList registerFieldList;
        MainWindow::getListRegisterFields(&registerFieldList);
        if(ui->label_cl_infoedit_title->text() == "Register Client")
        {

            if (dbManager->insertClientWithPic(&registerFieldList, &profilePic))
            {
                qDebug() << "Client registered successfully";
                clear_client_register_form();
                ui->stackedWidget->setCurrentIndex(1);
            }
            else
            {
                qDebug() << "Could not register client";
            }
        }
        else
        {
            if (dbManager->updateClientWithPic(&registerFieldList, curClientID, &profilePic))
            {
                qDebug() << "Client info edit successfully";
                clear_client_register_form();
                ui->stackedWidget->setCurrentIndex(1);
            }
            else
            {
                qDebug() << "Could not edit client info";
            }
        }

    }
    else
    {
        qDebug() << "Register form check was false";
    }
}


//check if the value is valid or not
bool MainWindow::check_client_register_form(){
    if(ui->lineEdit_cl_fName->text().isEmpty()
            && ui->lineEdit_cl_mName->text().isEmpty()
            && ui->lineEdit_cl_lName->text().isEmpty()){
        ui->lineEdit_cl_fName->setText("anonymous");
    }

    return true;
}


void MainWindow::defaultRegisterOptions(){
    //add caseWorker Name

    if(ui->comboBox_cl_caseWorker->findText("NONE")==-1){
        ui->comboBox_cl_caseWorker->addItem("NONE");
    }

    if(caseWorkerUpdated){
        QString caseWorkerquery = "SELECT Username, EmpId FROM Employee WHERE Role = 'CASE WORKER' ORDER BY Username";
        QSqlQuery caseWorkers = dbManager->execQuery(caseWorkerquery);
        //dbManager->printAll(caseWorkers);
        while(caseWorkers.next()){
         //   qDebug()<<"CASEWORKER: " <<caseWorkers.value(0).toString() << caseWorkers.value(1).toString();
            caseWorkerList.insert(caseWorkers.value(0).toString(), caseWorkers.value(1).toInt());
            if(ui->comboBox_cl_caseWorker->findText(caseWorkers.value(0).toString())==-1){
                ui->comboBox_cl_caseWorker->addItem(caseWorkers.value(0).toString());
            }
        }
        caseWorkerUpdated = false;
    }
    if(ui->comboBox_cl_status->findText("Green")==-1){
        ui->comboBox_cl_status->addItem("Green");
        ui->comboBox_cl_status->addItem("Yellow");
        ui->comboBox_cl_status->addItem("Red");
    }

}


/*==============================================================================
SEARCH CLIENTS USING NAME
==============================================================================*/
//search client
void MainWindow::on_pushButton_search_client_clicked()
{
    qDebug() <<"START SEARCH CLIENT";
    ui->tabWidget_cl_info->setCurrentIndex(0);
    QString clientName = ui->lineEdit_search_clientName->text();
    QString searchQuery = "SELECT ClientId, FirstName, LastName, Dob, Balance FROM Client WHERE LastName LIKE '%"+clientName
                        + "%' OR MiddleName Like '%"+ clientName
                        + "%' OR FirstName Like '%"+clientName+"%'";
    //QSqlQuery results = dbManager->execQuery(searchQuery);
    //setup_searchClientTable(results);

    QSqlQuery resultQ = dbManager->searchClientList(clientName);
    /*
    if(!(dbManager->searchClientList(&resultQ, clientName)))
    {
        qDebug()<<"Select Fail";
        return;
    }
    */
    //dbManager->printAll(resultQ);
    setup_searchClientTable(resultQ);

    connect(ui->tableWidget_search_client, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(selected_client_info(int,int)),Qt::UniqueConnection);
    // dbManager->printAll(results);

}


void MainWindow::setup_searchClientTable(QSqlQuery results){

    ui->tableWidget_search_client->setRowCount(0);

    int colCnt = results.record().count();
    ui->tableWidget_search_client->setColumnCount(colCnt);
    ui->tableWidget_search_client->clear();

    ui->tableWidget_search_client->setHorizontalHeaderLabels(QStringList()<<"ClientID"<<"FirstName"<<"Middle Initial"<<"LastName"<<"DateOfBirth"<<"Balance");
    int row =0;
    while(results.next()){
        ui->tableWidget_search_client->insertRow(row);
        for(int i =0; i<colCnt; i++){
            ui->tableWidget_search_client->setItem(row, i, new QTableWidgetItem(results.value(i).toString()));
            //qDebug() <<"row : "<<row << ", col: " << i << "item" << results.value(i).toString();
        }
        row++;
    }
    ui->tableWidget_search_client->show();

}




//get client information after searching

void MainWindow::selected_client_info(int nRow)
{


    if(!pic_available || !table_available)
        return;
    if(displayFuture.isRunning()|| !displayFuture.isFinished()){
        qDebug()<<"ProfilePic Is RUNNING";
        return;
        //displayFuture.cancel();
    }
    if(displayPicFuture.isRunning() || !displayPicFuture.isFinished()){
        qDebug()<<"ProfilePic Is RUNNING";
         return;
       // displayPicFuture.cancel();
    }
    ui->tabWidget_cl_info->setCurrentIndex(0);
    curClientID = ui->tableWidget_search_client->item(nRow, 0)->text();
    transacNum = 5;
    ui->pushButton_cl_trans_more->setEnabled(true);

    table_available = false;
    displayFuture = QtConcurrent::run(this, &displayClientInfoThread, curClientID);
    displayFuture.waitForFinished();
    displayPicFuture = QtConcurrent::run(this, &displayPicThread);
    displayPicFuture.waitForFinished();

}


void MainWindow::clientSearchedInfo(){

    QGraphicsScene *scene = new QGraphicsScene();
    scene->clear();
    ui->graphicsView_getInfo->setScene(scene);
}

void MainWindow::displayClientInfoThread(QString val){

    qDebug()<<"DISPLAY THREAD: " <<val;

    QSqlQuery clientInfo = dbManager->searchClientInfo(val);
//    QString searchQuery = "SELECT FirstName, MiddleName, LastName, Dob, Balance, SinNo, GaNo, DateRulesSigned, status FROM Client WHERE ClientId =" + val;

    // QString searchQuery = "SELECT FirstName, MiddleName, LastName, Dob, Balance FROM Client WHERE ClientId =" + val;
//    QSqlQuery clientInfoR = dbManager->execQuery(searchQuery);


   clientInfo.next();

   ui->label_cl_info_fName_val->setText(clientInfo.value(0).toString());
   ui->label_cl_info_mName_val->setText(clientInfo.value(1).toString());
   ui->label_cl_info_lName_val->setText(clientInfo.value(2).toString());
   ui->label_cl_info_dob_val->setText(clientInfo.value(3).toString());
   ui->label_cl_info_balance_amt->setText(clientInfo.value(4).toString());
   ui->label_cl_info_sin_val->setText(clientInfo.value(5).toString());
   ui->label_cl_info_gaNum_val->setText(clientInfo.value(6).toString());
   QString caseWorkerName = caseWorkerList.key(clientInfo.value(7).toInt());
   ui->label_cl_info_caseWorker_val->setText(caseWorkerName);
   ui->label_cl_info_ruleSignDate_val->setText(clientInfo.value(8).toString());
   ui->label_cl_info_status->setText(clientInfo.value(9).toString());

   ui->label_cl_info_nok_name_val->setText(clientInfo.value(10).toString());
   ui->label_cl_info_nok_relationship_val->setText(clientInfo.value(11).toString());
   ui->label_cl_info_nok_loc_val->setText(clientInfo.value(12).toString());
   ui->label_cl_info_nok_contatct_val->setText(clientInfo.value(13).toString());

   ui->label_cl_info_phys_name_val->setText(clientInfo.value(14).toString());
   ui->label_cl_info_phys_contact_val->setText(clientInfo.value(15).toString());

   ui->label_cl_info_Supporter_name_val->setText(clientInfo.value(16).toString());
   ui->label_cl_info_Supporter_contact_val->setText(clientInfo.value(17).toString());

   ui->label_cl_info_Supporter2_name_val->setText(clientInfo.value(18).toString());
   ui->label_cl_info_Supporter2_contact_val->setText(clientInfo.value(19).toString());
   ui->label_cl_info_comment->setText(clientInfo.value(20).toString());


// WITHOUT PICTURE
   /*
   QByteArray a = clientInfo.value(21).toByteArray();
   qDebug()<< "asdfa" <<a;
   profilePic =  QImage::fromData(a, "PNG");
*/
/*
   ui->label_cl_info_status->setText(clientInfo.value(8).toString());
   if(clientInfo.value(8).toString() == "green"){
       ui->label_cl_info_status->setStyleSheet("color: rgb(0, 204, 102);");
   }else if(clientInfo.value(8).toString() == "Yellow"){
       ui->label_cl_info_status->setStyleSheet("color: rgb(255, 255, 0);");
   }else if(clientInfo.value(8).toString() == "Red"){
       ui->label_cl_info_status->setStyleSheet("color: rgb(255, 0, 0);");

   }
*/


   table_available = true;



}

void MainWindow::displayPicThread()
{
    qDebug()<<"displayPicThread";

    if(!dbManager->searchClientInfoPic(&profilePic, curClientID)){
            qDebug()<<"ERROR to get pic";
            return;
        }
    qDebug()<<"Add picture";

    addInfoPic(profilePic);

   // QSqlQuery testQuery = dbManager->execQuery("SELECT  ProfilePic FROM Client WHERE ClientId = "+ curClientID);
   // testQuery.next();
   // QByteArray test = testQuery.value(0).toByteArray();
  //  profilePic = QImage::fromData(test, "PNG");
    // QImage profile = QImage::fromData(a, "PNG");
    /*
    QPixmap item2 = QPixmap::fromImage(profilePic);
    QPixmap scaled = QPixmap(item2.scaledToWidth((int)(ui->graphicsView_getInfo->width()*0.9), Qt::SmoothTransformation));
    QGraphicsScene *scene2 = new QGraphicsScene();
    scene2->addPixmap(QPixmap(scaled));
    ui->graphicsView_getInfo->setScene(scene2);
    ui->graphicsView_getInfo->show();
    pic_available=true;

*/
}


void MainWindow::addInfoPic(QImage img){
    qDebug()<<"Add Info Picture??";
    QPixmap item2 = QPixmap::fromImage(img);
    QPixmap scaled = QPixmap(item2.scaledToWidth((int)(ui->graphicsView_getInfo->width()*0.9), Qt::SmoothTransformation));
    QGraphicsScene *scene2 = new QGraphicsScene();
    scene2->addPixmap(QPixmap(scaled));
    ui->graphicsView_getInfo->setScene(scene2);
    ui->graphicsView_getInfo->show();
    pic_available=true;
}


void MainWindow::setSelectedClientInfo(){

    curClient = new Client();
    int nRow = ui->tableWidget_search_client->currentRow();
    if (nRow <0)
        return;

    curClientID = curClient->clientId = ui->tableWidget_search_client->item(nRow, 0)->text();
    curClient->fName =  ui->tableWidget_search_client->item(nRow, 1)->text();
    curClient->mName =  ui->tableWidget_search_client->item(nRow, 2)->text();
    curClient->lName =  ui->tableWidget_search_client->item(nRow, 3)->text();
    curClient->balance =  ui->tableWidget_search_client->item(nRow, 4)->text().toFloat();

    curClient->fullName = QString(curClient->fName + " " + curClient->mName + " " + curClient->lName);

}



void MainWindow::on_tabWidget_cl_info_currentChanged(int index)
{
    switch(index){
        case 0:
            qDebug()<<"Client information tab";
            break;

        case 1:
            if(transacFuture.isRunning()|| !transacFuture.isFinished()){
                qDebug()<<"ProfilePic Is RUNNING";
                return;
                //displayFuture.cancel();
            }
            ui->pushButton_cl_trans_more->setEnabled(true);
            transacFuture = QtConcurrent::run(this, &searchTransaction, curClientID);
            transacFuture.waitForFinished();
            qDebug()<<"client Transaction list";

            break;

    }
}



void MainWindow::searchTransaction(QString clientId){
    qDebug()<<"search transaction STaRt";
/*    QString transStr = QString("SELECT TOP 5 t.Date, t.Amount, t.Type, e.Username, t.ChequeNo, t.ChequeDate, t.TransType, t.Deleted ")
                     + QString("FROM Transac t INNER JOIN Employee e ON t.EmpId = e.EmpId ")
                     + QString("WHERE ClientId = " + clientId + " ORDER BY Date DESC");
    qDebug()<<transStr;
    QSqlQuery transQuery = dbManager->execQuery(transStr);
    */

    QSqlQuery transQuery = dbManager->searchClientTransList(transacNum, clientId);
    displayTransaction(transQuery);
    dbManager->printAll(transQuery);
    /*
            while(transQuery.next()){

    }
            */
}

void MainWindow::displayTransaction(QSqlQuery results){
    ui->tableWidget_transaction->setRowCount(0);

    int colCnt = results.record().count() -1;
    ui->tableWidget_transaction->setColumnCount(colCnt);
    ui->tableWidget_transaction->clear();

    ui->tableWidget_transaction->setHorizontalHeaderLabels(QStringList()<<"Date"<<"Amount"<<"Type"<<"Employee"<<"ChequeNo"<<"ChequeDate");
    int row =ui->tableWidget_transaction->rowCount();
    while(results.next()){
        ui->tableWidget_transaction->insertRow(row);
        for(int i =0; i<colCnt; i++){
            ui->tableWidget_transaction->setItem(row, i, new QTableWidgetItem(results.value(i).toString()));
            //qDebug() <<"row : "<<row << ", col: " << i << "item" << results.value(i).toString();
        }
        row++;
    }
    if(row !=transacNum || row%5 != 0){
        ui->pushButton_cl_trans_more->setEnabled(false);
    }
    ui->tableWidget_transaction->setMinimumHeight(33*row-5);

    ui->tableWidget_search_client->show();
}

void MainWindow::on_pushButton_cl_trans_more_clicked()
{

    transacNum +=5;
    searchTransaction(curClientID);
}




/////////////////////////////////////////////////////////////////////////////////


// the add user button
void MainWindow::on_btn_createNewUser_clicked()
{
    // temporary disable stuff
    // obtain username and pw and role from UI
    QString uname = ui->le_userName->text();
    QString pw = ui->le_password->text();

    if (uname.length() == 0) {
        ui->lbl_editUserWarning->setText("Enter a Username");
        return;
    }

    if (pw.length() == 0) {
        ui->lbl_editUserWarning->setText("Enter a Password");
        return;
    }

    // first, check to see if the username is taken
    QSqlQuery queryResults = dbManager->findUser(uname);
    int numrows = queryResults.numRowsAffected();

    if (numrows > 0) {
        ui->lbl_editUserWarning->setText("This username is already taken");
        return;
    } else {
        QSqlQuery queryResults = dbManager->addNewEmployee(uname, pw, ui->comboBox->currentText());
        int numrows = queryResults.numRowsAffected();

        if (numrows != 0) {
            ui->lbl_editUserWarning->setText("Employee added");
            QStandardItemModel * model = new QStandardItemModel(0,0);
            model->clear();
            ui->tableWidget_3->clear();
            ui->tableWidget_3->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidget_3->setColumnCount(3);
            ui->tableWidget_3->setRowCount(0);
            ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "Username" << "Password" << "Role");

            ui->comboBox->setCurrentIndex(0);
            ui->le_userName->setText("");
            ui->le_password->setText("");
            ui->le_users->setText("");
        } else {
            ui->lbl_editUserWarning->setText("Something went wrong - please try again");
        }
    }
}


void MainWindow::on_btn_dailyReport_clicked()
{
    ui->swdg_reports->setCurrentIndex(DAILYREPORT);
}

void MainWindow::on_btn_shiftReport_clicked()
{
    ui->swdg_reports->setCurrentIndex(SHIFTREPORT);
}

void MainWindow::on_btn_dailyLog_clicked()
{
    ui->swdg_reports->setCurrentIndex(DAILYLOG);
}

void MainWindow::on_btn_floatCount_clicked()
{
    ui->swdg_reports->setCurrentIndex(FLOATCOUNT);
}

void MainWindow::on_confirmationFinal_clicked()
{
    curBook = 0;
    curClient = 0;
    trans = 0;
    ui->stackedWidget->setCurrentIndex(MAINMENU);
}



void MainWindow::on_btn_listAllUsers_clicked()
{
    ui->tableWidget_3->setRowCount(0);
    ui->tableWidget_3->clear();
    ui->tableWidget_3->horizontalHeader()->setStretchLastSection(true);

    QSqlQuery result = dbManager->execQuery("SELECT Username, Password, Role FROM Employee");

    int numCols = result.record().count();
    ui->tableWidget_3->setColumnCount(numCols);
    ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "Username" << "Password" << "Role");
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        ui->tableWidget_3->insertRow(x);
        QStringList row;
        row << result.value(0).toString() << result.value(1).toString() << result.value(2).toString();
        for (int i = 0; i < 3; ++i)
        {
            ui->tableWidget_3->setItem(x, i, new QTableWidgetItem(row.at(i)));
        }
        x++;
    }
}

void MainWindow::on_btn_searchUsers_clicked()
{
    QString ename = ui->le_users->text();
    ui->tableWidget_3->setRowCount(0);
    ui->tableWidget_3->clear();
    ui->tableWidget_3->horizontalHeader()->setStretchLastSection(true);

    QSqlQuery result = dbManager->execQuery("SELECT Username, Password, Role FROM Employee WHERE Username LIKE '%"+ ename +"%'");

    int numCols = result.record().count();
    ui->tableWidget_3->setColumnCount(numCols);
    ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "Username" << "Password" << "Role");
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        ui->tableWidget_3->insertRow(x);
        QStringList row;
        row << result.value(0).toString() << result.value(1).toString() << result.value(2).toString();
        for (int i = 0; i < 3; ++i)
        {
            ui->tableWidget_3->setItem(x, i, new QTableWidgetItem(row.at(i)));
        }
        x++;
    }


//    QSqlQuery results = dbManager->execQuery("SELECT Username, Password, Role FROM Employee WHERE Username LIKE '%"+ ename +"%'");
//    QSqlQueryModel *model = new QSqlQueryModel();
//    model->setQuery(results);

////    ui->tableWidget_3->setModel(model);
////    ui->tableWidget_3->horizontalHeader()->model()->setHeaderData(0, Qt::Horizontal, "Username");
////    ui->tableWidget_3->horizontalHeader()->model()->setHeaderData(1, Qt::Horizontal, "Password");
////    ui->tableWidget_3->horizontalHeader()->model()->setHeaderData(2, Qt::Horizontal, "Role");
}




void MainWindow::initClientLookupInfo(){
    //init client search table
    if(ui->tableWidget_search_client->columnCount()>0){
        ui->tableWidget_search_client->setColumnCount(0);
        ui->tableWidget_search_client->clear();
    }

    //init client Info Form Field
    ui->label_cl_info_fName_val->clear();
    ui->label_cl_info_mName_val->clear();
    ui->label_cl_info_lName_val->clear();
    ui->label_cl_info_dob_val->clear();
    ui->label_cl_info_balance_amt->clear();
    ui->label_cl_info_sin_val->clear();
    ui->label_cl_info_gaNum_val->clear();
    ui->label_cl_info_caseWorker_val->clear();
    ui->label_cl_info_ruleSignDate_val->clear();
    ui->label_cl_info_status->clear();

    ui->label_cl_info_nok_name_val->clear();
    ui->label_cl_info_nok_relationship_val->clear();
    ui->label_cl_info_nok_loc_val->clear();
    ui->label_cl_info_nok_contatct_val->clear();

    ui->label_cl_info_phys_name_val->clear();
    ui->label_cl_info_phys_contact_val->clear();

    ui->label_cl_info_Supporter_name_val->clear();
    ui->label_cl_info_Supporter_contact_val->clear();
    ui->label_cl_info_Supporter2_name_val->clear();
    ui->label_cl_info_Supporter2_contact_val->clear();

    ui->label_cl_info_comment->clear();

    QGraphicsScene *scene = new QGraphicsScene();
    scene->clear();
    ui->graphicsView_getInfo->setScene(scene);

    profilePic = (QImage)NULL;

    //init client info table
    if(ui->tableWidget_clientInfo->columnCount()>0){
        ui->tableWidget_clientInfo->setColumnCount(0);
        ui->tableWidget_clientInfo->clear();
        ui->tableWidget_clientInfo2->setColumnCount(0);
        ui->tableWidget_clientInfo2->clear();
    }


}

// double clicked employee
void MainWindow::on_tableWidget_3_doubleClicked(const QModelIndex &index)
{
    // populate the fields on the right
    QString uname = ui->tableWidget_3->model()->data(ui->tableWidget_3->model()->index(index.row(), 0)).toString();
    QString pw = ui->tableWidget_3->model()->data(ui->tableWidget_3->model()->index(index.row(), 1)).toString();
    QString role = ui->tableWidget_3->model()->data(ui->tableWidget_3->model()->index(index.row(), 2)).toString();
    qDebug() << uname;
    qDebug() << pw;
    qDebug() << role;

//    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableWidget_3->model());
//    int row = index.row();

//     QStandardItemModel* model = ui->tableWidget_3->model();
//    qDebug() << model;
//    QString uname = model->item(row, 0)->text();
//    QString pw = model->item(row, 1)->text();
//    QString role = model->item(row, 2)->text();

    if (role == "STANDARD") {
        ui->comboBox->setCurrentIndex(0);
    } else if (role == "CASE WORKER") {
        ui->comboBox->setCurrentIndex(1);
    } else if (role == "ADMIN") {
        ui->comboBox->setCurrentIndex(2);
    }

    ui->le_userName->setText(uname);
    ui->le_password->setText(pw);
}

void MainWindow::on_pushButton_CaseFiles_clicked()
{
    addHistory(CLIENTLOOKUP);
    setSelectedClientInfo();
    ui->stackedWidget->setCurrentIndex(CASEFILE);

    double width = ui->tw_pcpRela->size().width();

    for (auto x: pcp_tables){
        x->resizeRowsToContents();
        x->setColumnWidth(0, width*0.41);
        x->setColumnWidth(1, width*0.41);
        x->setColumnWidth(2, width*0.16);
    }

    //get client id
    int nRow = ui->tableWidget_search_client->currentRow();
    if (nRow <0)
        return;
    curClientID = ui->tableWidget_search_client->item(nRow, 0)->text();
    QString curFirstName = ui->tableWidget_search_client->item(nRow, 1)->text();
    QString curMiddleName = ui->tableWidget_search_client->item(nRow, 2)->text().size() > 0 ? ui->tableWidget_search_client->item(nRow, 2)->text()+ " " : "";
    QString curLastName = ui->tableWidget_search_client->item(nRow, 3)->text();

    qDebug() << "id displayed:" << idDisplayed;
    qDebug() << "id selected:" << curClientID;
    if (idDisplayed != curClientID) {
        idDisplayed = curClientID;
        ui->lbl_caseClientName->setText(curFirstName + " " + curMiddleName + curLastName + "'s Case Files");
        populateCaseFiles();
    }
}

void MainWindow::populateCaseFiles(QString type, int tableId) {

    //running notes
    ui->te_notes->clear();
    QSqlQuery noteResult = dbManager->readNote(curClientID);
    qDebug() << noteResult.lastError();
    while (noteResult.next()) {
        ui->te_notes->document()->setPlainText(noteResult.value(0).toString());
    }

    //pcp tables
    int tableIdx = 0;
    if (type == "all") {

        for (auto x: pcpTypes) {
            QString query = "SELECT rowId, Goal, Strategy, Date "
                            "FROM Pcp "
                            "WHERE ClientId = " + curClientID +
                            " AND Type = '" + x + "'";
            QSqlQuery result = dbManager->execQuery(query);
            qDebug() << result.lastError();
            int numRows = result.numRowsAffected();
            auto table = (pcp_tables.at(tableIdx++));

            //reset table
            table->clearContents();
            table->setMinimumHeight(73);
            table->setMaximumHeight(1);
            table->setMaximumHeight(16777215);
            table->setRowCount(1);

            //set number of rows
            for (int i = 0; i < numRows-1; i++) {
                table->insertRow(0);

                //set height of table
                table->setMinimumHeight(table->minimumHeight() + 35);
            }

            //populate table
            while (result.next()){
                for (int i = 0; i < 3; i++) {
                    table->setItem(result.value(0).toString().toInt(), i, new QTableWidgetItem(result.value(i+1).toString()));
                }
            }
        }
        return;
    } else {

        QString query = "SELECT rowId, Goal, Strategy, Date "
                        "FROM Pcp "
                        "WHERE ClientId = " + curClientID +
                        " AND Type = '" + type + "'";
        QSqlQuery result = dbManager->execQuery(query);

        qDebug() << result.lastError();
        int numRows = result.numRowsAffected();
        auto table = (pcp_tables.at(tableId));

        //reset table
        table->clearContents();
        table->setMinimumHeight(73);
        table->setMaximumHeight(1);
        table->setMaximumHeight(16777215);
        table->setRowCount(1);

        //set number of rows
        for (int i = 0; i < numRows-1; i++) {
            table->insertRow(0);

            //set height of table
            table->setMinimumHeight(table->minimumHeight() + 35);
        }

        //populate table
        while (result.next()){
            for (int i = 0; i < 3; i++) {
                table->setItem(result.value(0).toString().toInt(), i, new QTableWidgetItem(result.value(i+1).toString()));
            }
        }
    }
}

// HANK
void MainWindow::on_EditRoomsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(EDITROOM);
    addHistory(ADMINPAGE);

    // set dropdowns
    populate_modRoom_cboxes();

    qDebug() << "pushed page " << ADMINPAGE;
}

// update employee button
void MainWindow::on_pushButton_4_clicked()
{
    // obtain username and pw and role from UI
    QString uname = ui->le_userName->text();
    QString pw = ui->le_password->text();

    if (uname.length() == 0) {
        ui->lbl_editUserWarning->setText("Enter a Username");
        return;
    }

    if (pw.length() == 0) {
        ui->lbl_editUserWarning->setText("Enter a Password");
        return;
    }

    // first, check to make sure the username is taken
    QSqlQuery queryResults = dbManager->findUser(uname);
    int numrows1 = queryResults.numRowsAffected();

    if (numrows1 == 1) {
        QSqlQuery queryResults = dbManager->updateEmployee(uname, pw, ui->comboBox->currentText());
        int numrows = queryResults.numRowsAffected();

        if (numrows != 0) {
            ui->lbl_editUserWarning->setText("Employee Updated");
            QStandardItemModel * model = new QStandardItemModel(0,0);
            model->clear();
            ui->tableWidget_3->clear();
            ui->tableWidget_3->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidget_3->setColumnCount(3);
            ui->tableWidget_3->setRowCount(0);
            ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "Username" << "Password" << "Role");

            ui->comboBox->setCurrentIndex(0);
            ui->le_userName->setText("");
            ui->le_password->setText("");
            ui->le_users->setText("");
        } else {
            ui->lbl_editUserWarning->setText("Something went wrong - Please try again");
        }

        return;
    } else {
        ui->lbl_editUserWarning->setText("Employee Not Found");
        return;
    }
}

// Clear button
void MainWindow::on_btn_displayUser_clicked()
{
    QStandardItemModel * model = new QStandardItemModel(0,0);
    model->clear();
    ui->tableWidget_3->clear();
    ui->tableWidget_3->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget_3->setColumnCount(3);
    ui->tableWidget_3->setRowCount(0);
    ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "Username" << "Password" << "Role");

    ui->comboBox->setCurrentIndex(0);
    ui->le_userName->setText("");
    ui->le_password->setText("");
    ui->le_users->setText("");
}

void MainWindow::on_tableWidget_3_clicked(const QModelIndex &index)
{
    // populate the fields on the right
    QString uname = ui->tableWidget_3->model()->data(ui->tableWidget_3->model()->index(index.row(), 0)).toString();
    QString pw = ui->tableWidget_3->model()->data(ui->tableWidget_3->model()->index(index.row(), 1)).toString();
    QString role = ui->tableWidget_3->model()->data(ui->tableWidget_3->model()->index(index.row(), 2)).toString();
    qDebug() << uname;
    qDebug() << pw;
    qDebug() << role;

//    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableWidget_3->model());
//    int row = index.row();

//     QStandardItemModel* model = ui->tableWidget_3->model();
//    qDebug() << model;
//    QString uname = model->item(row, 0)->text();
//    QString pw = model->item(row, 1)->text();
//    QString role = model->item(row, 2)->text();

    if (role == "STANDARD") {
        ui->comboBox->setCurrentIndex(0);
    } else if (role == "CASE WORKER") {
        ui->comboBox->setCurrentIndex(1);
    } else if (role == "ADMIN") {
        ui->comboBox->setCurrentIndex(2);
    }

    ui->le_userName->setText(uname);
    ui->le_password->setText(pw);
}

// delete button
void MainWindow::on_pushButton_6_clicked()
{
    QString uname = ui->le_userName->text();
    QString pw = ui->le_password->text();

    if (uname.length() == 0) {
        ui->lbl_editUserWarning->setText("Please make sure a valid employee is selected");
        return;
    }

    if (pw.length() == 0) {
        ui->lbl_editUserWarning->setText("Please make sure a valid employee is selected");
        return;
    }

    QSqlQuery queryResults = dbManager->findUser(uname);
    int numrows1 = queryResults.numRowsAffected();

    if (numrows1 == 1) {
        QSqlQuery queryResults = dbManager->deleteEmployee(uname, pw, ui->comboBox->currentText());
        int numrows = queryResults.numRowsAffected();

        if (numrows != 0) {
            ui->lbl_editUserWarning->setText("Employee Deleted");
            QStandardItemModel * model = new QStandardItemModel(0,0);
            model->clear();
            ui->tableWidget_3->clear();
            ui->tableWidget_3->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidget_3->setColumnCount(3);
            ui->tableWidget_3->setRowCount(0);
            ui->tableWidget_3->setHorizontalHeaderLabels(QStringList() << "Username" << "Password" << "Role");

            ui->comboBox->setCurrentIndex(0);
            ui->le_userName->setText("");
            ui->le_password->setText("");
            ui->le_users->setText("");
        } else {
            ui->lbl_editUserWarning->setText("Employee Not Found");
        }

        return;
    } else {
        ui->lbl_editUserWarning->setText("Employee Not Found");
        return;
    }

}

// list all rooms
void MainWindow::on_btn_listAllUsers_3_clicked()
{
    QString ename = ui->le_users_3->text();
    ui->tableWidget_5->setRowCount(0);
    ui->tableWidget_5->clear();
    ui->tableWidget_5->horizontalHeader()->setStretchLastSection(true);

    QSqlQuery result = dbManager->execQuery("SELECT SpaceCode, cost, Monthly FROM Space");

//    int numCols = result.record().count();
    ui->tableWidget_5->setColumnCount(8);
    ui->tableWidget_5->setHorizontalHeaderLabels(QStringList() << "ID Code" << "Building" << "Floor" << "Room" << "Bed Number" << "Type" << "Cost" << "Monthly");
    int x = 0;
    int qt = result.size();
    qDebug() << "<" << qt;
    while (result.next()) {
        // break down the spacecode

        QString spacecode = result.value(0).toString();
        if (spacecode == "") {
            break;
        }
        std::string strspacecode = spacecode.toStdString();

        std::vector<std::string> brokenupspacecode = split(strspacecode, '-');
        // parse space code to check building number + floor number + room number + space number
        QString buildingnum = QString::fromStdString(brokenupspacecode[0]);
        QString floornum = QString::fromStdString(brokenupspacecode[1]);
        QString roomnum = QString::fromStdString(brokenupspacecode[2]);
        std::string bednumtype = brokenupspacecode[3];
        // strip the last character
        QString bednumber = QString::fromStdString(bednumtype.substr(0, bednumtype.size()-1));

        // get the last character to figure out the type
        char typechar = bednumtype[bednumtype.size()-1];
        QString thetype = "" + typechar;

        ui->tableWidget_5->insertRow(x);
        QStringList row;
        row << spacecode
            << buildingnum
            << floornum
            << roomnum
            << bednumber
            << thetype
            << result.value(1).toString()
            << result.value(2).toString();
        for (int i = 0; i < 8; ++i)
        {
            ui->tableWidget_5->setItem(x, i, new QTableWidgetItem(row.at(i)));
        }
        x++;
    }
}

// list all programs
void MainWindow::on_btn_listAllUsers_2_clicked()
{
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->clear();
    ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);

    QSqlQuery result = dbManager->execQuery("SELECT ProgramCode, Description FROM Program");

    int numCols = result.record().count();
    ui->tableWidget_2->setColumnCount(numCols);
    ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "Program Code" << "Description");
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        ui->tableWidget_2->insertRow(x);
        QStringList row;
        row << result.value(0).toString() << result.value(1).toString();
        for (int i = 0; i < 2; ++i)
        {
            ui->tableWidget_2->setItem(x, i, new QTableWidgetItem(row.at(i)));
        }
        x++;
    }
}

// search programs by code
void MainWindow::on_btn_searchUsers_2_clicked()
{
    QString ename = ui->le_users_2->text();
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->clear();
    ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);

    QSqlQuery result = dbManager->execQuery("SELECT ProgramCode, Description FROM Program WHERE ProgramCode LIKE '%"+ ename +"%'");

    int numCols = result.record().count();
    ui->tableWidget_2->setColumnCount(numCols);
    ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "Program Code" << "Description");
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        ui->tableWidget_2->insertRow(x);
        QStringList row;
        row << result.value(0).toString() << result.value(1).toString();
        for (int i = 0; i < 2; ++i)
        {
            ui->tableWidget_2->setItem(x, i, new QTableWidgetItem(row.at(i)));
        }
        x++;
    }
}

// delete program
void MainWindow::on_pushButton_25_clicked()
{
    QString pcode = ui->le_userName_2->text();

    if (pcode.length() == 0) {
        ui->lbl_editUserWarning->setText("Please make sure a valid Program is selected");
        return;
    }

    QSqlQuery queryResults = dbManager->execQuery("DELETE FROM Program WHERE ProgramCode='" + pcode + "'");
    int numrows = queryResults.numRowsAffected();

    if (numrows != 0) {
        ui->lbl_editProgWarning->setText("Program Deleted");
        QStandardItemModel * model = new QStandardItemModel(0,0);
        model->clear();
        ui->tableWidget_2->clear();
        ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);
        ui->tableWidget_2->setColumnCount(2);
        ui->tableWidget_2->setRowCount(0);
        ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "Program Code" << "Description");

        ui->comboBox->setCurrentIndex(0);
        ui->le_userName->setText("");
        ui->le_password->setText("");
        ui->le_users->setText("");
    } else {
        ui->lbl_editProgWarning->setText("Program Not Found");
    }
    return;
}

// program clicked + selected
void MainWindow::on_tableWidget_2_clicked(const QModelIndex &index)
{
    if (lastprogramclicked != index) {
        ui->availablebedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
        ui->assignedbedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
        ui->lbl_editProgWarning->setText("Please hold while we set your beds");
        qApp->processEvents();

        ui->availablebedslist->clear();
        ui->availablebedslist->setRowCount(0);
        ui->assignedbedslist->clear();
        ui->assignedbedslist->setRowCount(0);

        ui->availablebedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
        ui->assignedbedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");

        // populate the fields on the right
        QString pcode = ui->tableWidget_2->model()->data(ui->tableWidget_2->model()->index(index.row(), 0)).toString();
        QString description = ui->tableWidget_2->model()->data(ui->tableWidget_2->model()->index(index.row(), 1)).toString();

        ui->le_userName_2->setText(pcode);
        ui->textEdit->setText(description);

        // populate the beds list
        QSqlQuery availSpaces = dbManager->getAvailableBeds(pcode);
        int numrowsavail = availSpaces.numRowsAffected();
        if (numrowsavail > 0) {
//            int numCols = availSpaces.record().count();
            ui->availablebedslist->setColumnCount(1);
            int x = 0;
            int qt = availSpaces.size();
            qDebug() << qt;
            while (availSpaces.next()) {
                ui->availablebedslist->insertRow(x);
                QStringList row;
                QString buildingNo = availSpaces.value(0).toString();
                QString floorNo = availSpaces.value(1).toString();
                QString roomNo = availSpaces.value(2).toString();
                QString spaceNo = availSpaces.value(4).toString();
                QString type = availSpaces.value(5).toString();

                //1-319-3
                QString roomCode = buildingNo + "-" + floorNo + "-" + roomNo + "-" + spaceNo + type[0];

                ui->availablebedslist->setItem(x, 0, new QTableWidgetItem(roomCode));
                x++;
            }
        }
        ui->availablebedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
        ui->assignedbedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
        qApp->processEvents();

    //    QStandardItemModel* availmodel = new QStandardItemModel();

    //    while (availSpaces.next()) {
    //        QString buildingNo = availSpaces.value(0).toString();
    //        QString floorNo = availSpaces.value(1).toString();
    //        QString roomNo = availSpaces.value(2).toString();
    //        QString spaceNo = availSpaces.value(4).toString();
    //        QString type = availSpaces.value(5).toString();

    //        //1-319-3
    //        QString roomCode = buildingNo + "-" + floorNo + "-" + roomNo + "-" + spaceNo + type[0];

    //        // QStandardItem* Items = new QStandardItem(availSpaces.value(1).toString());
    //        QStandardItem* Items = new QStandardItem(roomCode);
    //        availmodel->appendRow(Items);
    //    }

    //    ui->availablebedslist->setModel(availmodel);

          QSqlQuery assignedspaces = dbManager->getAssignedBeds(pcode);
          int numrowsassigned = assignedspaces.numRowsAffected();
          if (numrowsassigned > 0) {
//              int numCols = assignedspaces.record().count();
              ui->assignedbedslist->setColumnCount(1);
              int x = 0;
              int qt = assignedspaces.size();
              qDebug() << qt;
              while (assignedspaces.next()) {
                  ui->assignedbedslist->insertRow(x);
                  QStringList row;
                  QString buildingNo = assignedspaces.value(0).toString();
                  QString floorNo = assignedspaces.value(1).toString();
                  QString roomNo = assignedspaces.value(2).toString();
                  QString spaceNo = assignedspaces.value(4).toString();
                  QString type = assignedspaces.value(5).toString();

                  //1-319-3
                  QString roomCode = buildingNo + "-" + floorNo + "-" + roomNo + "-" + spaceNo + type[0];

                  ui->assignedbedslist->setItem(x, 0, new QTableWidgetItem(roomCode));
                  x++;
              }
          }
          ui->availablebedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
          ui->assignedbedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
          ui->lbl_editProgWarning->setText("");

    //    int numrowsassigned = assignedspaces.numRowsAffected();

    //    QStandardItemModel* assignedmodel = new QStandardItemModel();

    //    while (assignedspaces.next()) {
    //        QString buildingNo = assignedspaces.value(0).toString();
    //        QString floorNo = assignedspaces.value(1).toString();
    //        QString roomNo = assignedspaces.value(2).toString();
    //        QString spaceNo = assignedspaces.value(4).toString();
    //        QString type = assignedspaces.value(5).toString();

    //        //1-319-3
    //        QString roomCode = buildingNo + "-" + floorNo + "-" + roomNo + "-" + spaceNo + type[0];

    //        // QStandardItem* Items = new QStandardItem(availSpaces.value(1).toString());
    //        QStandardItem* Items = new QStandardItem(roomCode);
    //        assignedmodel->appendRow(Items);
    //    }

        // ui->assignedbedslist->setModel(assignedmodel);
          lastprogramclicked = index;
    }

}

// set case files directory
void MainWindow::on_pushButton_3_clicked()
{
    QString tempDir = QFileDialog::getExistingDirectory(
                    this,
                    tr("Select Directory"),
                    "C://"
                );
//    qDebug() << curClientID;
    if (!tempDir.isEmpty()) {
        dir = tempDir;
        populate_tw_caseFiles();
        ui->le_caseDir->setText(dir.path());
    }
    connect(ui->tw_caseFiles, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(on_tw_caseFiles_cellDoubleClicked(int,int)), Qt::UniqueConnection);
}

// open case file in external reader
void MainWindow::on_tw_caseFiles_cellDoubleClicked(int row, int column)
{
    qDebug() << QUrl::fromLocalFile(dir.absoluteFilePath(ui->tw_caseFiles->item(row, column)->text())) << "at" << row << " " << column;
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absoluteFilePath(ui->tw_caseFiles->item(row, column)->text())));
}

// filter file names
void MainWindow::on_btn_caseFilter_clicked()
{
    qDebug() << "filter button clicked, filter with" << ui->le_caseFilter->text();
    QStringList filter = (QStringList() << "*"+(ui->le_caseFilter->text())+"*");
    populate_tw_caseFiles(filter);
}

void MainWindow::populate_tw_caseFiles(QStringList filter){
    int i = 0;
    dir.setNameFilters(filter);
    for (auto x : dir.entryList(QDir::Files)) {
        qDebug() << x;
        ui->tw_caseFiles->setRowCount(i+1);
        ui->tw_caseFiles->setItem(i++,0, new QTableWidgetItem(x));
        ui->tw_caseFiles->resizeColumnsToContents();
    }
    if (i > 0) {
        ui->btn_caseFilter->setEnabled(true);
    }
}

// create new program button
void MainWindow::on_btn_createNewUser_2_clicked()
{
    QString pcode = ui->le_userName_2->text();
    QString pdesc = ui->textEdit->toPlainText();

    if (pcode.length() == 0) {
        ui->lbl_editProgWarning->setText("Please Enter a Program Code");
        return;
    }

    if (pdesc.length() == 0) {
        ui->lbl_editProgWarning->setText("Please Enter a Program Description");
        return;
    }

    QSqlQuery queryResults = dbManager->execQuery("SELECT * FROM Program WHERE ProgramCode='" + pcode + "'");
    int numrows = queryResults.numRowsAffected();

    if (numrows == 1) {
        ui->lbl_editProgWarning->setText("Program code in use");
        return;
    } else {
        QSqlQuery qr = dbManager->AddProgram(pcode, pdesc);
        if (qr.numRowsAffected() == 1) {
            ui->lbl_editProgWarning->setText("Program Added");
            QStandardItemModel * model = new QStandardItemModel(0,0);
            model->clear();
            ui->tableWidget_2->clear();
            ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidget_2->setColumnCount(2);
            ui->tableWidget_2->setRowCount(0);
            ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "Program Code" << "Description");

            ui->comboBox->setCurrentIndex(0);
            ui->le_userName->setText("");
            ui->le_password->setText("");
            ui->le_users->setText("");
        } else {
            ui->lbl_editProgWarning->setText("Program not added - please try again.");
        }
    }
}

// update program
void MainWindow::on_pushButton_24_clicked()
{
    QString pcode = ui->le_userName_2->text();
    QString pdesc = ui->textEdit->toPlainText();

    if (pcode.length() == 0) {
        ui->lbl_editProgWarning->setText("Please Enter a Program Code");
        return;
    }

    if (pdesc.length() == 0) {
        ui->lbl_editProgWarning->setText("Please Enter a Program Description");
        return;
    }

    QSqlQuery queryResults = dbManager->execQuery("SELECT * FROM Program WHERE ProgramCode='" + pcode + "'");
    int numrows = queryResults.numRowsAffected();

    if (numrows != 1) {
        ui->lbl_editProgWarning->setText("Enter a valid program code to update");
        return;
    } else {
        QSqlQuery qr = dbManager->updateProgram(pcode, pdesc);
        if (qr.numRowsAffected() == 1) {
            ui->lbl_editProgWarning->setText("Program Updated");
            QStandardItemModel * model = new QStandardItemModel(0,0);
            model->clear();
            ui->tableWidget_2->clear();
            ui->tableWidget_2->horizontalHeader()->setStretchLastSection(true);
            ui->tableWidget_2->setColumnCount(2);
            ui->tableWidget_2->setRowCount(0);
            ui->tableWidget_2->setHorizontalHeaderLabels(QStringList() << "Program Code" << "Description");

            ui->comboBox->setCurrentIndex(0);
            ui->le_userName->setText("");
            ui->le_password->setText("");
            ui->le_users->setText("");
        } else {
            ui->lbl_editProgWarning->setText("Program not updated - please try again.");
        }
    }
}


void MainWindow::resizeEvent() {
    double width = ui->tw_pcpRela->size().width();
    for (auto x: pcp_tables){
        x->resizeRowsToContents();
        x->setColumnWidth(0, width*0.41);
        x->setColumnWidth(1, width*0.41);
        x->setColumnWidth(2, width*0.16);
    }
}

//void MainWindow::on_tw_pcpRela_itemChanged(QTableWidgetItem *item)
//{
//    for (auto x: pcp_tables){
//        x->resizeRowsToContents();
//    }
//}

void MainWindow::initPcp(){
    pcp_tables.push_back(ui->tw_pcpRela);
    pcp_tables.push_back(ui->tw_pcpEdu);
    pcp_tables.push_back(ui->tw_pcpSub);
    pcp_tables.push_back(ui->tw_pcpAcco);
    pcp_tables.push_back(ui->tw_pcpLife);
    pcp_tables.push_back(ui->tw_pcpMent);
    pcp_tables.push_back(ui->tw_pcpPhy);
    pcp_tables.push_back(ui->tw_pcpLeg);
    pcp_tables.push_back(ui->tw_pcpAct);
    pcp_tables.push_back(ui->tw_pcpTrad);
    pcp_tables.push_back(ui->tw_pcpOther);
    pcp_tables.push_back(ui->tw_pcpPpl);

    pcpTypes = {
                    "relationship",
                    "educationEmployment",
                    "substanceUse",
                    "accomodationsPlanning",
                    "lifeSkills",
                    "mentalHealth",
                    "physicalHealth",
                    "legalInvolvement",
                    "activities",
                    "traditions",
                    "other",
                    "people"
                };
}

void MainWindow::on_btn_pcpRela_clicked()
{
    ui->tw_pcpRela->insertRow(0);
    ui->tw_pcpRela->setMinimumHeight(ui->tw_pcpRela->minimumHeight()+35);
}

void MainWindow::on_btn_pcpEdu_clicked()
{
    ui->tw_pcpEdu->insertRow(0);
    ui->tw_pcpEdu->setMinimumHeight(ui->tw_pcpEdu->minimumHeight()+35);
}

void MainWindow::on_btn_pcpSub_clicked()
{
    ui->tw_pcpSub->insertRow(0);
    ui->tw_pcpSub->setMinimumHeight(ui->tw_pcpSub->minimumHeight()+35);
}

void MainWindow::on_btn_pcpAcc_clicked()
{
    ui->tw_pcpAcco->insertRow(0);
    ui->tw_pcpAcco->setMinimumHeight(ui->tw_pcpAcco->minimumHeight()+35);
}

void MainWindow::on_btn_pcpLife_clicked()
{
    ui->tw_pcpLife->insertRow(0);
    ui->tw_pcpLife->setMinimumHeight(ui->tw_pcpLife->minimumHeight()+35);
}

void MainWindow::on_btn_pcpMent_clicked()
{
    ui->tw_pcpMent->insertRow(0);
    ui->tw_pcpMent->setMinimumHeight(ui->tw_pcpMent->minimumHeight()+35);
}

void MainWindow::on_btn_pcpPhy_clicked()
{
    ui->tw_pcpPhy->insertRow(0);
    ui->tw_pcpPhy->setMinimumHeight(ui->tw_pcpPhy->minimumHeight()+35);
}

void MainWindow::on_btn_pcpLeg_clicked()
{
    ui->tw_pcpLeg->insertRow(0);
    ui->tw_pcpLeg->setMinimumHeight(ui->tw_pcpLeg->minimumHeight()+35);
}

void MainWindow::on_btn_pcpAct_clicked()
{
    ui->tw_pcpAct->insertRow(0);
    ui->tw_pcpAct->setMinimumHeight(ui->tw_pcpAct->minimumHeight()+35);
}

void MainWindow::on_btn_pcpTrad_clicked()
{
    ui->tw_pcpTrad->insertRow(0);
    ui->tw_pcpTrad->setMinimumHeight(ui->tw_pcpTrad->minimumHeight()+35);
}

void MainWindow::on_btn_pcpOther_clicked()
{
    ui->tw_pcpOther->insertRow(0);
    ui->tw_pcpOther->setMinimumHeight(ui->tw_pcpOther->minimumHeight()+35);
}

void MainWindow::on_btn_pcpKey_clicked()
{
    ui->tw_pcpPpl->insertRow(0);
    ui->tw_pcpPpl->setMinimumHeight(ui->tw_pcpPpl->minimumHeight()+35);
}

void MainWindow::on_addbedtoprogram_clicked()
{
    // add program tag to bed
    QString pcode = ui->le_userName_2->text();

    // QModelIndexList qil = ui->availablebedslist->selectedIndexes();

    // get selected bed
    qDebug() << ui->availablebedslist->currentItem()->text();
    std::string selectedtag = ui->availablebedslist->currentItem()->text().toStdString();

    std::vector<std::string> tagbreakdown = split(selectedtag, '-');

    // parse space code to check building number + floor number + room number + space number
    QString buildingnum = QString::fromStdString(tagbreakdown[0]);
    QString floornum = QString::fromStdString(tagbreakdown[1]);
    QString roomnum = QString::fromStdString(tagbreakdown[2]);
    std::string bednumtype = tagbreakdown[3];
    // strip the last character
    QString bednumber = QString::fromStdString(bednumtype.substr(0, bednumtype.size()-1));

    // get space id
    QSqlQuery singlebedquery = dbManager->searchSingleBed(buildingnum, floornum, roomnum, bednumber);
    singlebedquery.next();
    QString spaceid = singlebedquery.value(3).toString();

    // get curr tag
    QString currenttag = singlebedquery.value(8).toString();

    // append to tag
    currenttag += ",";
    currenttag += pcode;

    qDebug() << currenttag;

    // update tag value
    dbManager->updateSpaceProgram(spaceid, currenttag);

    ui->availablebedslist->clear();
    ui->availablebedslist->setRowCount(0);
    ui->assignedbedslist->clear();
    ui->assignedbedslist->setRowCount(0);
    ui->availablebedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
    ui->assignedbedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
    ui->lbl_editProgWarning->setText("Bed Added to Program");

//    const auto selectedIdxs = ui->availablebedslist->selectedIndexes();
//    if (!selectedIdxs.isEmpty()) {
//        const QVariant var = ui->availablebedslist->model()->data(selectedIdxs.first());
//        // next you need to convert your `var` from `QVariant` to something that you show from your data with default (`Qt::DisplayRole`) role, usually it is a `QString`:
//        const QString selectedItemString = var.toString();

//        // or you also may do this by using `QStandardItemModel::itemFromIndex()` method:
//        const QStandardItem* selectedItem = dynamic_cast<QStandardItemModel*>(model())->itemFromIndex(selectedIdxs.first());
//        // use your `QStandardItem`
//    }

    // populate the beds list
//    QSqlQuery availSpaces = dbManager->getAvailableBeds(pcode);
//    int numrowsavail = availSpaces.numRowsAffected();

//    QStandardItemModel* availmodel = new QStandardItemModel();

//    while (availSpaces.next()) {
//        QString buildingNo = availSpaces.value(0).toString();
//        QString floorNo = availSpaces.value(1).toString();
//        QString roomNo = availSpaces.value(2).toString();
//        QString spaceNo = availSpaces.value(4).toString();
//        QString type = availSpaces.value(5).toString();

//        //1-319-3
//        QString roomCode = buildingNo + "-" + floorNo + "-" + roomNo + "-" + spaceNo + type[0];

//        // QStandardItem* Items = new QStandardItem(availSpaces.value(1).toString());
//        QStandardItem* Items = new QStandardItem(roomCode);
//        availmodel->appendRow(Items);
//    }

//    ui->availablebedslist->setModel(availmodel);


}

void MainWindow::on_removebedfromprogram_clicked()
{
    // remove program tag from bed
    QString pcode = ui->le_userName_2->text();

    // get selected bed
    qDebug() << ui->assignedbedslist->currentItem()->text();
    std::string selectedtag = ui->assignedbedslist->currentItem()->text().toStdString();

    std::vector<std::string> tagbreakdown = split(selectedtag, '-');

    // parse space code to check building number + floor number + room number + space number
    QString buildingnum = QString::fromStdString(tagbreakdown[0]);
    QString floornum = QString::fromStdString(tagbreakdown[1]);
    QString roomnum = QString::fromStdString(tagbreakdown[2]);
    std::string bednumtype = tagbreakdown[3];
    // strip the last character
    QString bednumber = QString::fromStdString(bednumtype.substr(0, bednumtype.size()-1));

    // get space id
    QSqlQuery singlebedquery = dbManager->searchSingleBed(buildingnum, floornum, roomnum, bednumber);
    singlebedquery.next();
    QString spaceid = singlebedquery.value(3).toString();

    // get curr tag
    QString currenttag = singlebedquery.value(8).toString();

    // remove tag
    QString newtag = "";
    std::string curtagtogether = currenttag.toStdString();
    std::vector<std::string> curtagbreakdown = split(curtagtogether, ',');
    for (std::string t : curtagbreakdown) {
        if (QString::fromStdString(t) != pcode) {
            newtag += QString::fromStdString(t);
            newtag += ",";
        }
    }
    std::string tempstr = newtag.toStdString();
    tempstr.pop_back();
    newtag = QString::fromStdString(tempstr);

    qDebug() << newtag;

    // update tag value
    dbManager->updateSpaceProgram(spaceid, newtag);
    ui->availablebedslist->clear();
    ui->availablebedslist->setRowCount(0);
    ui->assignedbedslist->clear();
    ui->assignedbedslist->setRowCount(0);
    ui->availablebedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
    ui->assignedbedslist->setHorizontalHeaderLabels(QStringList() << "Bed Code");
    ui->lbl_editProgWarning->setText("Bed Removed from Program");
}

void MainWindow::on_availablebedslist_clicked(const QModelIndex &index)
{
    // clicked available bed
//    availableBedIndex = index;
//    QStringList list;
//    QStandardItemModel model = (QStandardItemModel) ui->availablebedslist->model();
//    foreach(const QModelIndex &index, ui->availablebedslist->selectionModel()->selectedIndexes()) {
//        list.append(model->itemFromIndex(index)->text());
//    }
}

void MainWindow::on_assignedbedslist_clicked(const QModelIndex &index)
{
    // clicked assigned bed
    assignedBedIndex = index;
}

void MainWindow::on_btn_monthlyReport_clicked()
{
    ui->swdg_reports->setCurrentIndex(MONTHLYREPORT);
}


void MainWindow::on_btn_restrictedList_clicked()
{
    ui->swdg_reports->setCurrentIndex(RESTRICTIONS);
}


/*==============================================================================
REPORTS
==============================================================================*/
void MainWindow::setupReportsScreen()
{
    ui->lbl_dailyReportDateVal->setText(QDate::currentDate().toString("yyyy-MM-dd"));
    ui->dailyReport_dateEdit->setDate(QDate::currentDate());
    checkoutReport = new Report(this, ui->checkout_tableView, CHECKOUT_REPORT);
    vacancyReport = new Report(this, ui->vacancy_tableView, VACANCY_REPORT);
    lunchReport = new Report(this, ui->lunch_tableView, LUNCH_REPORT);
    wakeupReport = new Report(this, ui->wakeup_tableView, WAKEUP_REPORT);
    bookingReport = new Report(this, ui->booking_tableView, BOOKING_REPORT);
    transactionReport = new Report(this, ui->transaction_tableView, TRANSACTION_REPORT);
}

void MainWindow::updateDailyReportTables(QDate date)
{
    useProgressDialog("Processing reports...", QtConcurrent::run(checkoutReport, &checkoutReport->updateModelThread, date));

//    checkoutReport->updateModel(date);
    vacancyReport->updateModel(date);
    lunchReport->updateModel(date);
    wakeupReport->updateModel(date);
    ui->lbl_dailyReportDateVal->setText(date.toString("yyyy-MM-dd"));
    ui->dailyReport_dateEdit->setDate(date);

    
}

void MainWindow::updateShiftReportTables(QDate date, int shiftNo)
{
    qDebug() << "ShiftNO in updateshiftreport tables " << shiftNo;
    bookingReport->updateModel(date, shiftNo);
    transactionReport->updateModel(date, shiftNo);
}

void MainWindow::on_dailyReportGo_btn_clicked()
{
    QDate date = ui->dailyReport_dateEdit->date();
    MainWindow::updateDailyReportTables(date);
    MainWindow::getDailyReportStats(date);
}


void MainWindow::on_dailyReportCurrent_btn_clicked()
{
    ui->dailyReport_dateEdit->setDate(QDate::currentDate());
}

void MainWindow::on_shiftReportGo_btn_clicked()
{

}

void MainWindow::on_shiftReportCurrent_btn_clicked()
{

}

void MainWindow::getDailyReportStats(QDate date)
{
    qDebug() << "getDailyReportStats  called";
    QtConcurrent::run(dbManager, &DatabaseManager::getDailyReportStatsThread, date);
}

void MainWindow::updateDailyReportStats(QList<int> list)
{
    ui->lbl_espCheckouts->setText(QString::number(list.at(0)));
    ui->lbl_totalCheckouts->setText(QString::number(list.at(1)));
    ui->lbl_espVacancies->setText(QString::number(list.at(2)));
    ui->lbl_totalVacancies->setText(QString::number(list.at(3)));
}
/*==============================================================================
REPORTS (END)
==============================================================================*/

void MainWindow::on_actionBack_triggered()
{
    if (!backStack.isEmpty()){
        int page = backStack.pop();
        forwardStack.push(ui->stackedWidget->currentIndex());
        ui->stackedWidget->setCurrentIndex(page);
        ui->actionForward->setEnabled(true);
    }
}

void MainWindow::on_actionForward_triggered()
{
    if (!forwardStack.isEmpty()) {
        int page = forwardStack.pop();
        backStack.push(ui->stackedWidget->currentIndex());
        ui->stackedWidget->setCurrentIndex(page);
    }
}

void MainWindow::addHistory(int n){
    backStack.push(n);
    forwardStack.clear();
    ui->actionBack->setEnabled(true);
    ui->actionForward->setEnabled(false);
}

void MainWindow::on_pushButton_processPaymeent_clicked()
{
    addHistory(CLIENTLOOKUP);
}

void MainWindow::insertPcp(QTableWidget *tw, QString type){
    QString goal;
    QString strategy;
    QString date;

    int rows = tw->rowCount();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 3; j++) {
            if (tw->item(i,j)) {
                switch (j) {
                case 0: goal = tw->item(i,j)->text();
                    break;
                case 1: strategy = tw->item(i,j)->text();
                    break;
                case 2: date = tw->item(i,j)->text();
                }

              }
        }
        QSqlQuery delResult = dbManager->deletePcpRow(i, type);
        qDebug() << delResult.lastError();
        QSqlQuery addResult = dbManager->addPcp(i, curClientID, type, goal, strategy, date);
        qDebug() << addResult.lastError();
    }
}

void MainWindow::on_btn_pcpRelaSave_clicked()
{
    insertPcp(ui->tw_pcpRela, "relationship");
}

void MainWindow::on_btn_pcpEduSave_clicked()
{
    insertPcp(ui->tw_pcpEdu, "educationEmployment");
}

void MainWindow::on_btn_pcpSubSave_clicked()
{
    insertPcp(ui->tw_pcpSub, "substanceUse");
}

void MainWindow::on_btn_pcpAccoSave_clicked()
{
    insertPcp(ui->tw_pcpAcco, "accomodationsPlanning");
}

void MainWindow::on_btn_pcpLifeSave_clicked()
{
    insertPcp(ui->tw_pcpLife, "lifeSkills");
}

void MainWindow::on_btn_pcpMentSave_clicked()
{
    insertPcp(ui->tw_pcpMent, "mentalHealth");
}

void MainWindow::on_btn_pcpPhySave_clicked()
{
    insertPcp(ui->tw_pcpPhy, "physicalHealth");
}

void MainWindow::on_btn_pcpLegSave_clicked()
{
    insertPcp(ui->tw_pcpLeg, "legalInvolvement");
}

void MainWindow::on_btn_pcpActSave_2_clicked()
{
    insertPcp(ui->tw_pcpAct, "activities");
}

void MainWindow::on_btn_pcpTradSave_clicked()
{
    insertPcp(ui->tw_pcpTrad, "traditions");
}

void MainWindow::on_btn_pcpOtherSave_clicked()
{
    insertPcp(ui->tw_pcpOther, "other");
}

void MainWindow::on_btn_pcpKeySave_clicked()
{
    insertPcp(ui->tw_pcpPpl, "people");
}

void MainWindow::on_actionPcptables_triggered()
{

}

void MainWindow::on_btn_pcpRelaUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(0);
    populateCaseFiles(pcpTypes.at(0), 0);
}

void MainWindow::on_btn_pcpEduUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(1);
    populateCaseFiles(pcpTypes.at(1), 1);
}

void MainWindow::on_btn_pcpSubUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(2);
    populateCaseFiles(pcpTypes.at(2), 2);
}

void MainWindow::on_btn_pcpAccoUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(3);
    populateCaseFiles(pcpTypes.at(3), 3);
}

void MainWindow::on_btn_pcpLifeUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(4);
    populateCaseFiles(pcpTypes.at(4), 4);
}

void MainWindow::on_btn_pcpMentUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(5);
    populateCaseFiles(pcpTypes.at(5), 5);
}

void MainWindow::on_btn_pcpPhyUndo_2_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(6);
    populateCaseFiles(pcpTypes.at(6), 6);
}

void MainWindow::on_btn_pcpLegUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(7);
    populateCaseFiles(pcpTypes.at(7), 7);
}

void MainWindow::on_btn_pcpActUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(8);
    populateCaseFiles(pcpTypes.at(8), 8);
}

void MainWindow::on_btn_pcpTradUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(9);
    populateCaseFiles(pcpTypes.at(9), 9);
}

void MainWindow::on_btn_pcpOtherUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(10);
    populateCaseFiles(pcpTypes.at(10), 10);
}

void MainWindow::on_btn_pcpPplUndo_clicked()
{
    qDebug() << "resetting table " << pcpTypes.at(11);
    populateCaseFiles(pcpTypes.at(11), 11);
}

void MainWindow::on_btn_notesSave_clicked()
{
    QString notes = ui->te_notes->toPlainText();
    QSqlQuery result = dbManager->addNote(curClientID, notes);
    if (result.numRowsAffected() == 1) {
//        ui->lbl_noteWarning->setStyleSheet("QLabel#lbl_noteWarning {color = black;}");
//        ui->lbl_noteWarning->setText("Saved");
    } else {
//        ui->lbl_noteWarning->setStyleSheet("QLabel#lbl_noteWarning {color = red;}");
//        ui->lbl_noteWarning->setText(result.lastError().text());
        QSqlQuery result2 = dbManager->updateNote(curClientID, notes);
        qDebug() << result2.lastError();

    }

}

void MainWindow::on_btn_notesUndo_clicked()
{
    QSqlQuery result = dbManager->readNote(curClientID);
    qDebug() << result.lastError();
    while (result.next()) {
        ui->te_notes->document()->setPlainText(result.value(0).toString());
    }
}

// UNTESTED
void MainWindow::on_btn_searchUsers_3_clicked()
{
    QString ename = ui->le_users_3->text();
    ui->tableWidget_5->setRowCount(0);
    ui->tableWidget_5->clear();
    ui->tableWidget_5->horizontalHeader()->setStretchLastSection(true);

    QSqlQuery result = dbManager->execQuery("SELECT SpaceCode, cost, Montly FROM Space WHERE SpaceCode LIKE '%"+ ename +"%'");

//    int numCols = result.record().count();
    ui->tableWidget_5->setColumnCount(8);
    ui->tableWidget_5->setHorizontalHeaderLabels(QStringList() << "ID Code" << "Building" << "Floor" << "Room" << "Bed Number" << "Type" << "Cost" << "Monthly");
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        // break down the spacecode

        QString spacecode = result.value(0).toString();
        std::string strspacecode = spacecode.toStdString();

        std::vector<std::string> brokenupspacecode = split(strspacecode, '-');
        // parse space code to check building number + floor number + room number + space number
        QString buildingnum = QString::fromStdString(brokenupspacecode[0]);
        QString floornum = QString::fromStdString(brokenupspacecode[1]);
        QString roomnum = QString::fromStdString(brokenupspacecode[2]);
        std::string bednumtype = brokenupspacecode[3];
        // strip the last character
        QString bednumber = QString::fromStdString(bednumtype.substr(0, bednumtype.size()-1));

        // get the last character to figure out the type
        char typechar = bednumtype[bednumtype.size()-1];
        QString thetype = "" + typechar;

    // get space id
//    QSqlQuery singlebedquery = dbManager->searchSingleBed(buildingnum, floornum, roomnum, bednumber);
//    singlebedquery.next();
//    QString spaceid = singlebedquery.value(3).toString();

        ui->tableWidget_5->insertRow(x);
        QStringList row;
        row << spacecode
            << buildingnum
            << floornum
            << roomnum
            << bednumber
            << thetype
            << result.value(1).toString()
            << result.value(2).toString();
        for (int i = 0; i < 8; ++i)
        {
            ui->tableWidget_5->setItem(x, i, new QTableWidgetItem(row.at(i)));
        }
        x++;
    }
}

void MainWindow::populate_modRoom_cboxes() {
    ui->cbox_roomLoc->clear();
    ui->cbox_roomFloor->clear();
    ui->cbox_roomRoom->clear();
    ui->cbox_roomType->clear();

    QSqlQuery buildings = dbManager->execQuery("SELECT BuildingNo FROM Building");
    ui->cbox_roomLoc->addItems(QStringList() << "");
    while (buildings.next()) {
        ui->cbox_roomLoc->addItems(QStringList() << buildings.value(0).toString());
    }
}

void MainWindow::on_btn_modRoomBdlg_clicked()
{
    curmodifyingspace = BUILDINGS;
    ui->editroommodifybox->clear();
    ui->editroommodifybox->setColumnCount(1);
    ui->editroommodifybox->setRowCount(0);
    ui->editroommodifybox->horizontalHeader()->setStretchLastSection(true);
    ui->editroommodifybox->setHorizontalHeaderLabels(QStringList() << "Building Number");

    // get all buildings
    QSqlQuery result = dbManager->execQuery("SELECT BuildingNo FROM Building");

    // populate table
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        QString buildingno = result.value(0).toString();
        ui->editroommodifybox->insertRow(x);
        QStringList row;
        row << buildingno;
        for (int i = 0; i < 1; ++i)
        {
            ui->editroommodifybox->setItem(x, i, new QTableWidgetItem(row.at(i)));
            ui->editroommodifybox->item(x, i)->setTextAlignment(Qt::AlignCenter);
        }
        x++;
    }
}

void MainWindow::on_btn_modRoomFloor_clicked()
{
    curmodifyingspace = FLOORS;
    ui->editroommodifybox->clear();
    ui->editroommodifybox->setColumnCount(1);
    ui->editroommodifybox->setRowCount(0);
    ui->editroommodifybox->horizontalHeader()->setStretchLastSection(true);
    ui->editroommodifybox->setHorizontalHeaderLabels(QStringList() << "Floor Number");
    // get building id
    QString building = ui->cbox_roomLoc->currentText();
    QSqlQuery qry = dbManager->execQuery("SELECT BuildingId FROM Building WHERE BuildingNo=" + building);

    qry.next();
    QString buildingid = qry.value(0).toString();

    QSqlQuery result = dbManager->execQuery("SELECT FloorNo FROM Floor WHERE BuildingId=" + buildingid);

    // populate table
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        QString buildingno = result.value(0).toString();
        ui->editroommodifybox->insertRow(x);
        QStringList row;
        row << buildingno;
        for (int i = 0; i < 1; ++i)
        {
            ui->editroommodifybox->setItem(x, i, new QTableWidgetItem(row.at(i)));
            ui->editroommodifybox->item(x, i)->setTextAlignment(Qt::AlignCenter);
        }
        x++;
    }
}

void MainWindow::on_btn_modRoomRoom_clicked()
{
    curmodifyingspace = ROOMS;
    ui->editroommodifybox->clear();
    ui->editroommodifybox->setColumnCount(1);
    ui->editroommodifybox->setRowCount(0);
    ui->editroommodifybox->horizontalHeader()->setStretchLastSection(true);
    ui->editroommodifybox->setHorizontalHeaderLabels(QStringList() << "Room Number");

    // get building id
    QString building = ui->cbox_roomLoc->currentText();
    QSqlQuery qry = dbManager->execQuery("SELECT BuildingId FROM Building WHERE BuildingNo=" + building);

    qry.next();
    QString buildingid = qry.value(0).toString();

    // get Floor id
    QString floor = ui->cbox_roomFloor->currentText();
    QSqlQuery qry2 = dbManager->execQuery("SELECT FloorId FROM Floor WHERE BuildingId=" + building + " AND FloorNo=" + floor);

    qry2.next();

    QString floorid = qry2.value(0).toString();

    QSqlQuery result = dbManager->execQuery("SELECT RoomNo FROM Room WHERE FloorId=" + floorid);

    // populate table
    int x = 0;
    int qt = result.size();
    qDebug() << qt;
    while (result.next()) {
        QString buildingno = result.value(0).toString();
        ui->editroommodifybox->insertRow(x);
        QStringList row;
        row << buildingno;
        for (int i = 0; i < 1; ++i)
        {
            ui->editroommodifybox->setItem(x, i, new QTableWidgetItem(row.at(i)));
            ui->editroommodifybox->item(x, i)->setTextAlignment(Qt::AlignCenter);
        }
        x++;
    }
}

void MainWindow::on_btn_modRoomType_clicked()
{
    curmodifyingspace = TYPE;
    // this shouldn't be modifiable... - there are only ever beds and mats.
}

void MainWindow::on_EditShiftsButton_clicked()
{
    addHistory(ADMINPAGE);
    ui->stackedWidget->setCurrentIndex(EDITSHIFT);
}

void MainWindow::on_cbox_roomLoc_currentTextChanged(const QString &arg1)
{
    qDebug() << arg1;
    // clear the other boxes
    // ui->cbox_roomLoc->clear();
    ui->cbox_roomFloor->clear();
    ui->cbox_roomRoom->clear();
    ui->cbox_roomType->clear();

    if (arg1 == "") {
        // empty selected, exit function
        return;
    }

    // get building id
    QString building = ui->cbox_roomLoc->currentText();
    QSqlQuery qry = dbManager->execQuery("SELECT BuildingId FROM Building WHERE BuildingNo=" + arg1);

    qry.next();

    // populate floor numbers based on selected building
    QSqlQuery floors = dbManager->execQuery("SELECT FloorNo FROM Floor WHERE BuildingId=" + qry.value(0).toString());

    ui->cbox_roomFloor->addItems(QStringList() << "");
    while (floors.next()) {
        ui->cbox_roomFloor->addItems(QStringList() << floors.value(0).toString());
        qDebug() << "added item" + floors.value(0).toString();
    }
}

void MainWindow::on_cbox_roomFloor_currentTextChanged(const QString &arg1)
{
    // ui->cbox_roomLoc->clear();
    // ui->cbox_roomFloor->clear();
    ui->cbox_roomRoom->clear();
    ui->cbox_roomType->clear();

    if (arg1 == "") {
        // empty selected, exit function
        return;
    }

    // get building id
    QString building = ui->cbox_roomLoc->currentText();
    QSqlQuery qry = dbManager->execQuery("SELECT BuildingId FROM Building WHERE BuildingNo=" + building);

    qry.next();

    qDebug() << "val:" << qry.value(0).toString();

    // populate room numbers based on selected floor
    // get floor id
    QSqlQuery qry2 = dbManager->execQuery("SELECT FloorId FROM Floor WHERE BuildingId=" + qry.value(0).toString() + " AND FloorNo=" + arg1);

    qry2.next();

    QString floorid = qry2.value(0).toString();
    qDebug() << "floorid:" + floorid;

    QSqlQuery rooms = dbManager->execQuery("SELECT RoomNo FROM Room WHERE FloorId=" + floorid);

    ui->cbox_roomRoom->addItems(QStringList() << "");
    while (rooms.next()) {
        ui->cbox_roomRoom->addItems(QStringList() << rooms.value(0).toString());
    }
}

void MainWindow::on_cbox_roomRoom_currentTextChanged(const QString &arg1)
{
    // ui->cbox_roomLoc->clear();
    // ui->cbox_roomFloor->clear();
    // ui->cbox_roomRoom->clear();
    ui->cbox_roomType->clear();

    if (arg1 == "") {
        // empty selected, exit function
        return;
    }

    QSqlQuery types = dbManager->execQuery("SELECT TypeDesc FROM Type");
    while (types.next()) {
        ui->cbox_roomType->addItems(QStringList() << types.value(0).toString());
    }
}

void MainWindow::on_cbox_roomType_currentTextChanged(const QString &arg1)
{

}

void MainWindow::on_cbox_roomType_currentIndexChanged(int index)
{

}


void MainWindow::on_tableWidget_search_client_itemClicked()
{
    ui->pushButton_bookRoom->setEnabled(true);
    ui->pushButton_processPaymeent->setEnabled(true);
    ui->pushButton_editClientInfo->setEnabled(true);
    ui->pushButton_CaseFiles->setEnabled(true);
}


void MainWindow::on_programDropdown_currentIndexChanged()
{
    clearTable(ui->bookingTable);
    ui->makeBookingButton->hide();
}

void MainWindow::on_confirmAddLunch_clicked()
{
    MyCalendar* mc = new MyCalendar(this, curBook->startDate,curBook->endDate, curClient,1);
       mc->exec();
}

void MainWindow::on_confirmAddWake_clicked()
{
    MyCalendar* mc = new MyCalendar(this, curBook->startDate,curBook->endDate, curClient,2);
        mc->exec();
}

void MainWindow::on_editLunches_clicked()
{
    MyCalendar* mc;
    if(QDate::currentDate() < curBook->startDate){
        mc = new MyCalendar(this, curBook->startDate, curBook->endDate, curClient,1);


    }else{
       mc = new MyCalendar(this, QDate::currentDate(), curBook->endDate, curClient,1);
    }
       mc->exec();
}

void MainWindow::on_editWakeup_clicked()
{
    MyCalendar* mc;
    if(QDate::currentDate() < curBook->startDate){

        mc = new MyCalendar(this, curBook->startDate,curBook->endDate, curClient,2);
    }else{

        mc = new MyCalendar(this, QDate::currentDate(),curBook->endDate, curClient,2);
    }
        mc->exec();
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::useProgressDialog(QString msg, QFuture<void> future){
    dialog->setLabelText(msg);
    futureWatcher.setFuture(future);
    dialog->setCancelButton(0);
    dialog->exec();
    futureWatcher.waitForFinished();
}
