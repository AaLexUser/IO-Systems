#include "cpu.h"
#include "display.h"
#include "spi_master.h"

using namespace std;

CPU::CPU(sc_module_name nm)
    : sc_module(nm), clk_i("clk_i"), addr_bo("addr_bo"), data_bi("data_bi"), data_bo("data_bo"), wr_o("wr_o"),
      rd_o("rd_o")
{
    addr_bo.initialize(0);
    data_bo.initialize(0);
    wr_o.initialize(0);
    rd_o.initialize(0);

    SC_CTHREAD(mainThread, clk_i.pos());
}

CPU::~CPU()
{
}

void CPU::mainThread()
{

    // spi_write(DISP_SYMBOL_ADDR, '>');
    // spi_write(DISP_CURSOR_ADDR, 5);
    // spi_write(DISP_SYMBOL_ADDR, 'H');
    // spi_write(DISP_SYMBOL_ADDR, 'e');
    // spi_write(DISP_SYMBOL_ADDR, 'l');
    // spi_write(DISP_SYMBOL_ADDR, 'l');
    // spi_write(DISP_SYMBOL_ADDR, 'o');
    // spi_write(DISP_CURSOR_ADDR, DISPLAY_SIZE-1);
    // spi_write(DISP_SYMBOL_ADDR, '<');
    // spi_write(DISP_CMD_REG_ADDR, DISP_UPDATE_CMD);

    // spi_write(DISP_CURSOR_ADDR, 10);
    // spi_write(DISP_SYMBOL_ADDR, '!');
    // spi_write(DISP_CMD_REG_ADDR, DISP_UPDATE_CMD);

    show_forecast(25, 50, 756);


    sc_stop();
}

void CPU::spi_write(int addr, int data)
{
    int wd_ctr = 0;
    bool busy_flag = false;
    int data_send = (addr << SPI_ADDR_SIZE) | (data & 0x00FF);

    // send data
    bus_write(DATA_SEND_ADDR, data_send);

    // start trans
    bus_write(CMD_REG_ADDR, CMD_SEND_BIT);

    busy_flag = bus_read(CMD_REG_ADDR) & CMD_BUSY_BIT;
    while (busy_flag)
    {
        wd_ctr++;
        if (wd_ctr == 100)
            break;
        busy_flag = bus_read(CMD_REG_ADDR) & CMD_BUSY_BIT;
    }
}

int CPU::spi_read(int addr)
{
}

int CPU::bus_read(int addr)
{
    int data;

    wait();
    addr_bo.write(addr);
    rd_o.write(1);

    wait();
    rd_o.write(0);

    wait();
    data = data_bi.read();

    // cout << "CPU: READ " << endl;
    // cout << "  -> addr: " << hex << addr << endl;
    // cout << "  -> data: " << hex << data << endl;

    return data;
}

void CPU::bus_write(int addr, int data)
{
    wait();
    addr_bo.write(addr);
    data_bo.write(data);
    wr_o.write(1);

    wait();
    wr_o.write(0);

    // cout << "CPU: WRITE " << endl;
    // cout << "  -> addr: " << hex << addr << endl;
    // cout << "  -> data: " << hex << data << endl;
}

void CPU::show_forecast(int temperature, int humidity, int pressure)
{
    char temp_str[DISPLEY_LINE_SIZE];
    char hum_str[DISPLEY_LINE_SIZE];
    char press_str[DISPLEY_LINE_SIZE];
    snprintf(temp_str, DISPLEY_LINE_SIZE, "T: %d", temperature);
    snprintf(hum_str, DISPLEY_LINE_SIZE, "H: %d", humidity);
    snprintf(press_str, DISPLEY_LINE_SIZE, "P: %d", pressure);

    for (int i = 0; i < DISPLEY_LINE_SIZE && temp_str[i] != '\0'; i++)
    {
        spi_write(DISP_SYMBOL_ADDR, temp_str[i]);
    }

    spi_write(DISP_CURSOR_ADDR, DISPLEY_LINE_SIZE);

    for (int i = 0; i < DISPLEY_LINE_SIZE && hum_str[i] != '\0'; i++)
    {
        spi_write(DISP_SYMBOL_ADDR, hum_str[i]);
    }

    spi_write(DISP_CURSOR_ADDR, DISPLEY_LINE_SIZE * 2);

    for (int i = 0; i < DISPLEY_LINE_SIZE && press_str[i] != '\0'; i++)
    {
        spi_write(DISP_SYMBOL_ADDR, press_str[i]);
    }

    spi_write(DISP_CMD_REG_ADDR, DISP_UPDATE_CMD);
}
