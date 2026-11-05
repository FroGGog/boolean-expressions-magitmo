# Boolean Expressions: Parser, AST and Validation

## О задаче

Нужно реализовать библиотеку, которая принимает набор именованных булевых
выражений, разбирает их в abstract syntax tree (AST) и проверяет корректность
ссылок между выражениями.

Такие компоненты встречаются в rule engines, системах управления доступом,
feature flags, фильтрах, конфигурациях политик и обработчиках событий. Выражения
часто приходят в виде текста, но исполнять непроверенный текст напрямую нельзя:
сначала необходимо проверить синтаксис, разрешить имена, найти отсутствующие
зависимости и отвергнуть циклы.

Задача объединяет несколько самостоятельных инженерных тем:

- tokenizer и recursive-descent parser;
- проектирование immutable AST;
- таблицу символов и разрешение имён;
- построение dependency graph;
- поиск циклов с понятной диагностикой;
- проверку сложного кода через positive и negative tests.

Результатом будет законченный frontend маленького декларативного языка. Его
можно использовать как основу для отдельного следующего этапа — компиляции AST
в компактный и быстрый evaluation plan.

Ориентировочный бюджет: **9–13 часов**. Вариант с подробными diagnostics,
точными source spans и расширенным набором тестов: **12–16 часов**.

## Что находится в репозитории

- минимальный CMake-проект на C++20;
- фиксированный публичный API;
- начальная заглушка `BuildProgram()`;
- пример использования в `examples/main.cc`;
- public valid/invalid fixtures в `tests/public_cases.h`;
- GoogleTest 1.17.0 и автоматическое обнаружение тестов через CTest.

Tokenizer, parser, AST и алгоритм проверки зависимостей не реализованы.

## Публичный контракт

### Вход

Вход задаётся через C++ API. Реализовывать JSON/YAML parser или дополнительный
формат конфигурации не требуется.

```cpp
boolean_expressions::ProgramSource source{
    .inputs = {
        "has_token",
        "office_network",
        "malware_detected",
    },
    .expressions = {
        {"employee", "has_token | office_network"},
        {"trusted", "employee & !malware_detected"},
        {"allow", "trusted & has_token"},
    },
    .outputs = {
        "allow",
    },
};
```

- `inputs` — имена внешних значений типа `bool`;
- `expressions` — пары из имени выражения и его текста;
- `outputs` — имена выражений, считающихся результатами программы.

`inputs` и `expressions` находятся в одном namespace. Ссылка внутри выражения
может указывать либо на input, либо на другое именованное expression. Forward
references разрешены: выражение может ссылаться на выражение, объявленное ниже.

### Выход

```cpp
using BuildResult =
    std::variant<ValidatedProgram, std::vector<Diagnostic>>;
```

При успехе возвращается `ValidatedProgram`, владеющий immutable AST всех
выражений. Внутреннее представление AST не задаётся публичным API, но должно
содержать пять логических видов nodes:

- boolean literal;
- reference;
- unary `NOT`;
- binary `AND`;
- binary `OR`.

При ошибке возвращается непустой список `Diagnostic`. Частично валидная
программа возвращаться не должна. Допустимо прекратить обработку после первой
ошибки. Если возвращается несколько ошибок, их порядок должен быть
детерминированным.

Минимальные категории ошибок:

- syntax error;
- duplicate name;
- collision между именем input и expression;
- unknown reference;
- unknown output;
- dependency cycle.

## Что такое SourceSpan

`SourceSpan` указывает на фрагмент исходного текста, связанный с AST node или
ошибкой:

```cpp
struct SourceSpan {
  std::size_t begin;
  std::size_t end;
};
```

Используется полуинтервал `[begin, end)`: `begin` включён, `end` не включён.
Индексы считаются от начала текста одного выражения. Грамматика использует
ASCII identifiers, поэтому индексы bytes и символов совпадают.

Пример:

```text
a & missing
01234567890
    ^^^^^^^
```

Для неизвестной ссылки `missing` span равен `[4, 11)`. Это позволяет показать
ошибку рядом с исходным выражением:

```text
unknown reference "missing"
a & missing
    ^^^^^^^
```

Для неожиданного token `|` в `a & | b` span равен `[4, 5)`. Ошибка в конце
строки, например отсутствующий operand после `a &`, может использовать
zero-width span `[3, 3)`.

Некоторые diagnostics не относятся к конкретному фрагменту текста: unknown
output, duplicate input name или общий dependency cycle. Поэтому поле имеет тип
`std::optional<SourceSpan>` и для таких ошибок может быть пустым.

## Грамматика

```ebnf
expression         = or_expression ;
or_expression      = and_expression, { "|", and_expression } ;
and_expression     = unary_expression, { "&", unary_expression } ;
unary_expression   = "!", unary_expression
                   | primary_expression ;
primary_expression = identifier
                   | boolean_literal
                   | "(", expression, ")" ;
boolean_literal    = "true" | "false" ;
identifier         = letter_or_underscore,
                     { letter_or_digit_or_underscore } ;
```

Идентификатор соответствует регулярному выражению:

```text
[A-Za-z_][A-Za-z0-9_]*
```

`true` и `false` — зарезервированные слова. Они не могут использоваться как
имена inputs или expressions. Пробельные символы между tokens игнорируются.

Приоритет операций от высокого к низкому:

1. `(...)`;
2. `!`;
3. `&`;
4. `|`.

`&` и `|` левоассоциативны. `!` применяется к следующему unary expression,
поэтому `!!a` является корректным выражением.

Например:

```text
!has_token | !office_network & !s1
```

эквивалентно:

```text
(!has_token) | ((!office_network) & (!s1))
```

## Что именно требуется реализовать

### 1. Tokenizer

Tokenizer должен полностью разобрать строку выражения в tokens:

- identifier;
- `true` и `false`;
- `!`, `&`, `|`;
- `(` и `)`;
- end-of-input.

Каждый token должен хранить `SourceSpan`. Неизвестный символ должен приводить к
`kSyntaxError`, а не игнорироваться.

### 2. Parser и AST

Нужно реализовать parser согласно указанной грамматике. Подходящая реализация —
recursive descent с отдельным методом для каждого уровня приоритета.

Parser должен:

- учитывать приоритет и associativity операторов;
- поддерживать вложенные скобки;
- потреблять строку целиком;
- отвергать пустое выражение и trailing input;
- строить immutable AST;
- сохранять source spans AST nodes или хотя бы leaf references и literals.

### 3. Проверка имён

До проверки зависимостей нужно построить единую таблицу символов.

Требуется обнаруживать:

- повторное объявление input;
- повторное объявление expression;
- одинаковое имя у input и expression;
- использование `true` или `false` как имени;
- output, не соответствующий ни одному именованному expression.

Output должен ссылаться именно на expression, а не непосредственно на input.

### 4. Разрешение references

Каждый `Reference` в AST должен быть разрешён в input или named expression.
Неизвестное имя приводит к `kUnknownReference`; diagnostic должен содержать имя
выражения и span проблемного reference.

Forward references разрешены, поэтому разрешение имён нельзя ограничивать уже
прочитанными definitions.

### 5. Dependency graph и циклы

Для каждой ссылки expression-to-expression нужно добавить dependency edge.
Ссылки на inputs рёбер между expressions не создают.

Необходимо обнаруживать:

- self-cycle `a -> a`;
- цикл из двух expressions;
- цикл произвольной длины;
- цикл внутри одной из нескольких независимых компонент.

Подойдёт DFS с состояниями `unvisited`, `visiting`, `visited` или эквивалентный
алгоритм. Для ошибки нужно вернуть конкретный замкнутый путь:

```text
a -> b -> c -> a
```

Сообщения вида `cycle found` без пути недостаточно.

### 6. ValidatedProgram и DebugString

Если все проверки прошли, нужно создать `ValidatedProgram`, который владеет AST
и не зависит от времени жизни `ProgramSource`.

`ValidatedProgram::DebugString()` возвращает expressions в порядке исходного
`ProgramSource` в детерминированной prefix-форме:

```text
employee = (OR (REF has_token) (REF office_network))
trusted = (AND (REF employee) (NOT (REF malware_detected)))
allow = (AND (REF trusted) (REF has_token))
```

Каждая строка заканчивается `\n`. Формат nodes:

```text
(LITERAL true)
(LITERAL false)
(REF name)
(NOT child)
(AND left right)
(OR left right)
```

Prefix-форма позволяет проверять структуру AST через публичный API, не
фиксируя конкретную иерархию внутренних C++ классов.

### 7. Тесты и краткое описание решения

Нужно заменить начальные wiring tests полноценными unit tests и использовать
все cases из `tests/public_cases.h`. Тесты пишутся на GoogleTest; отдельный
`main()` не нужен, так как target связан с `GTest::gtest_main`.

Дополнительно должны быть покрыты:

- приоритеты и associativity;
- несколько последовательных `!`;
- глубоко вложенные скобки;
- whitespace между любыми tokens;
- пустая строка и пустые скобки;
- незакрытая скобка;
- отсутствующий operand;
- неожиданный token;
- trailing input;
- duplicate names и reserved words;
- unknown reference и unknown output;
- self-cycle и цикл из нескольких expressions;
- корректный DAG;
- несколько независимых компонент.

В итоговом README нужно кратко описать:

- выбранное представление AST и ownership;
- устройство tokenizer/parser;
- алгоритм разрешения имён;
- алгоритм поиска циклов;
- дополнительные принятые решения.

## Пример результата

Для входа:

```cpp
.inputs = {"has_token", "office_network", "malware_detected"},
.expressions = {
    {"employee", "has_token | office_network"},
    {"trusted", "employee & !malware_detected"},
    {"allow", "trusted & has_token"},
},
.outputs = {"allow"},
```

ожидается успешный результат:

```text
employee = (OR (REF has_token) (REF office_network))
trusted = (AND (REF employee) (NOT (REF malware_detected)))
allow = (AND (REF trusted) (REF has_token))
```

Для входа:

```cpp
.inputs = {"input"},
.expressions = {
    {"a", "b & input"},
    {"b", "!c"},
    {"c", "a | input"},
},
.outputs = {"a"},
```

ожидается `kDependencyCycle` с путём:

```text
a -> b -> c -> a
```

## Public cases и дополнительные проверки

`tests/public_cases.h` содержит стартовый набор:

- valid programs с ожидаемой prefix-формой AST;
- invalid programs с ожидаемым `DiagnosticCode`;
- ожидаемые пути для cyclic dependencies.

Cases представлены C++ fixtures, поэтому вспомогательный parser тестового
JSON/YAML не нужен. Дополнительные black-box cases могут подключаться через тот
же публичный `ProgramSource` и `BuildResult`.

## Что не входит в задачу

- вычисление выражений для конкретных bool values;
- JSON/YAML parser;
- `&&`, `||`, сравнения, числа, строки и функции;
- error recovery после syntax error;
- оптимизация или constant folding AST;
- common subexpression elimination;
- компиляция в dense evaluation plan;
- benchmarks и выбор runtime containers;
- параллельное или incremental evaluation.

## Сборка

### Linux, macOS и Ninja на Windows

Первый configure загружает закреплённый GoogleTest 1.17.0 с GitHub:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Один конкретный test можно запустить напрямую:

```bash
./build/boolean_expressions_tests \
  --gtest_filter=Starter.PublicCasesAreAvailable
```

На Windows с multi-config Visual Studio generator:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

И прямой запуск:

```powershell
.\build\Debug\boolean_expressions_tests.exe `
  --gtest_filter=Starter.PublicCasesAreAvailable
```

Для окружения с уже установленным CMake package GoogleTest загрузку можно
отключить:

```bash
cmake -S . -B build \
  -DBUILD_TESTING=ON \
  -DBOOLEAN_EXPRESSIONS_USE_SYSTEM_GTEST=ON
```

Если тесты не нужны, `-DBUILD_TESTING=OFF` не загружает GoogleTest.

GoogleTest подключается по URL архива, поэтому наличие `git` для configure не
требуется. Требуется HTTPS-доступ к GitHub либо заранее установленный GTest.

### Платформенная совместимость

Проект не использует платформенные API и рассчитан на:

- Linux с GCC или Clang, поддерживающим C++20;
- Windows с Visual Studio 2022/MSVC либо актуальным MinGW/Clang;
- macOS с актуальным AppleClang;
- CMake 3.20 или новее.

Для Windows установлен `gtest_force_shared_crt=ON`: GoogleTest использует тот же
вариант MSVC runtime, что и основной проект, и не создаёт конфликт `/MD` с
`/MT`. Для Linux thread flags и libraries приходят транзитивно через
`GTest::gtest_main`; вручную добавлять `-pthread` не требуется.

`gtest_discover_tests()` работает и с single-config generators (Ninja, Make), и
с multi-config Visual Studio generator. Для Visual Studio при запуске CTest
нужно указывать ту же конфигурацию через `-C Debug` или `-C Release`.

Основной внешний риск одинаков для Linux и Windows: `FetchContent` должен иметь
HTTPS-доступ к GitHub во время первого configure. В offline или корпоративном
окружении следует установить GoogleTest заранее и включить
`BOOLEAN_EXPRESSIONS_USE_SYSTEM_GTEST=ON` либо предоставить CMake локальный
FetchContent source cache.

Запуск примера:

```bash
./build/boolean_expressions_example
```

До реализации `BuildProgram()` пример сообщает:

```text
TODO: BuildProgram is not implemented
```

После реализации он печатает prefix-форму построенного AST.

## Критерии готовности

- проект собирается указанными командами;
- публичный API из `include/` остаётся совместимым;
- tokenizer возвращает tokens с корректными source spans;
- parser строит AST согласно грамматике, приоритетам и associativity;
- все references разрешаются через единую таблицу символов;
- неизвестные и конфликтующие имена диагностируются;
- любой dependency cycle отвергается с конкретным замкнутым путём;
- при ошибке не возвращается partially valid program;
- `DebugString()` имеет детерминированный формат;
- public cases и добавленные boundary tests проходят;
- решение и ключевые алгоритмы кратко описаны.
