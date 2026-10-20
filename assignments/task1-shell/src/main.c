#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int hello_cmd(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    shell_print(sh, "Hello from my BIE-IOT application!");
    return 0;
}

SHELL_CMD_REGISTER(hello, NULL, "Print a greeting", hello_cmd);

int main(void)
{
    return 0;
}
