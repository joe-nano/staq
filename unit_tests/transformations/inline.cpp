/*
 * This file is part of staq.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gtest/gtest.h"
#include "parser/parser.hpp"
#include "transformations/inline.hpp"

using namespace staq;

// Testing inlining
/******************************************************************************/
TEST(Inline, Simple) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo(x) q {\n"
                      "\tU(x,x,x) q;\n"
                      "}\n"
                      "qreg q[1];\n"
                      "foo(0) q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "gate foo(x) q {\n"
                       "\tU(x,x,x) q;\n"
                       "}\n"
                       "qreg q[1];\n"
                       "U(0,0,0) q[0];\n";

    auto program = parser::parse_string(pre, "simple.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/

/******************************************************************************/
TEST(Inline, Multi_Level) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo(x) q {\n"
                      "\tU(x,x,x) q;\n"
                      "}\n"
                      "gate bar p {\n"
                      "\tfoo(pi) p;\n"
                      "}\n"
                      "qreg q[1];\n"
                      "bar q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "gate foo(x) q {\n"
                       "\tU(x,x,x) q;\n"
                       "}\n"
                       "gate bar p {\n"
                       "\tU(pi,pi,pi) p;\n"
                       "}\n"
                       "qreg q[1];\n"
                       "U(pi,pi,pi) q[0];\n";

    auto program = parser::parse_string(pre, "multi_level.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/

/******************************************************************************/
TEST(Inline, Multi_Ancilla) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo q {\n"
                      "\tancilla a[1];\n"
                      "\tancilla b[1];\n"
                      "\tCX q,a[0];\n"
                      "\tCX q,b[0];\n"
                      "}\n"
                      "qreg q[1];\n"
                      "foo q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "qreg anc[2];\n"
                       "gate foo q {\n"
                       "\tancilla a[1];\n"
                       "\tancilla b[1];\n"
                       "\tCX q,a[0];\n"
                       "\tCX q,b[0];\n"
                       "}\n"
                       "qreg q[1];\n"
                       "CX q[0],anc[0];\n"
                       "CX q[0],anc[1];\n";

    auto program = parser::parse_string(pre, "multi_ancilla.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/

/******************************************************************************/
TEST(Inline, Dirty_Ancilla) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo q {\n"
                      "\tdirty ancilla a[1];\n"
                      "\tCX q,a[0];\n"
                      "}\n"
                      "qreg q[2];\n"
                      "foo q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "gate foo q {\n"
                       "\tdirty ancilla a[1];\n"
                       "\tCX q,a[0];\n"
                       "}\n"
                       "qreg q[2];\n"
                       "CX q[0],q[1];\n";

    auto program = parser::parse_string(pre, "dirty_ancilla.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/

/******************************************************************************/
TEST(Inline, Dirty_Ancilla_No_Free) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo q {\n"
                      "\tdirty ancilla a[1];\n"
                      "\tCX q,a[0];\n"
                      "}\n"
                      "qreg q[1];\n"
                      "foo q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "qreg anc[1];\n"
                       "gate foo q {\n"
                       "\tdirty ancilla a[1];\n"
                       "\tCX q,a[0];\n"
                       "}\n"
                       "qreg q[1];\n"
                       "CX q[0],anc[0];\n";

    auto program = parser::parse_string(pre, "dirty_ancilla_no_free.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/

/******************************************************************************/
TEST(Inline, Dirty_Ancilla_Split) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo q {\n"
                      "\tdirty ancilla a[2];\n"
                      "\tCX q,a[0];\n"
                      "\tCX q,a[1];\n"
                      "}\n"
                      "qreg q[2];\n"
                      "qreg r[1];\n"
                      "foo q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "gate foo q {\n"
                       "\tdirty ancilla a[2];\n"
                       "\tCX q,a[0];\n"
                       "\tCX q,a[1];\n"
                       "}\n"
                       "qreg q[2];\n"
                       "qreg r[1];\n"
                       "CX q[0],q[1];\n"
                       "CX q[0],r[0];\n";

    auto program = parser::parse_string(pre, "dirty_ancilla_split.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/

/******************************************************************************/
TEST(Inline, Mixed_Ancilla) {
    std::string pre = "OPENQASM 2.0;\n"
                      "\n"
                      "gate foo q {\n"
                      "\tancilla a[1];\n"
                      "\tdirty ancilla b[1];\n"
                      "\tCX q,a[0];\n"
                      "\tCX q,b[0];\n"
                      "}\n"
                      "qreg q[1];\n"
                      "foo q[0];\n";

    std::string post = "OPENQASM 2.0;\n"
                       "\n"
                       "qreg anc[2];\n"
                       "gate foo q {\n"
                       "\tancilla a[1];\n"
                       "\tdirty ancilla b[1];\n"
                       "\tCX q,a[0];\n"
                       "\tCX q,b[0];\n"
                       "}\n"
                       "qreg q[1];\n"
                       "CX q[0],anc[0];\n"
                       "CX q[0],anc[1];\n";

    auto program = parser::parse_string(pre, "mixed_ancilla.qasm");
    transformations::inline_ast(*program);
    std::stringstream ss;
    ss << *program;

    EXPECT_EQ(ss.str(), post);
}
/******************************************************************************/
