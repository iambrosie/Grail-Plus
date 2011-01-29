/*
 * ContextFreeGrammar.hpp
 *
 *  Created on: Sep 19, 2010
 *      Author: Peter Goodman
 *     Version: $Id$
 */

#ifndef FLTL_LIB_CONTEXTFREEGRAMMAR_HPP_
#define FLTL_LIB_CONTEXTFREEGRAMMAR_HPP_

#include <cassert>
#include <cstring>
#include <map>
#include <utility>
#include <stdint.h>

#include "fltl/include/helper/Align.hpp"
#include "fltl/include/helper/Array.hpp"
#include "fltl/include/helper/BlockAllocator.hpp"
#include "fltl/include/helper/StorageChain.hpp"
#include "fltl/include/helper/UnsafeCast.hpp"

#include "fltl/include/mpl/If.hpp"
#include "fltl/include/mpl/Static.hpp"

#include "fltl/include/preprocessor/CATENATE.hpp"
#include "fltl/include/preprocessor/FORCE_INLINE.hpp"
#include "fltl/include/preprocessor/REPEAT_LEFT.hpp"

#include "fltl/include/trait/Uncopyable.hpp"

/// the class name of a query let'ed variable
#define FLTL_CFG_LET_VAR_CLASS_NAME(var_name) \
    cfg_var__ ## var_name ## __

#define FLTL_CFG_LET_VAR_OPAQUE_CLASS_NAME(var_name) \
    cfg_var__ ## var_name ## __opaque

/// macro for building query variables to be used specifically for
/// CFG queries.
#define FLTL_CFG_LET_(type,var_name) \
    typedef struct FLTL_CFG_LET_VAR_OPAQUE_CLASS_NAME(var_name) *FLTL_CFG_LET_VAR_CLASS_NAME(var_name); \
    fltl::mpl::Expr< \
        type, \
        FLTL_CFG_LET_VAR_CLASS_NAME(var_name) \
    > var_name

/// macros for building (non-)terminal query variables
#define FLTL_CFG_LET_NON_TERM(var_name) FLTL_CFG_LET_(fltl::lib::cfg::query_non_terminal_tag, var_name)
#define FLTL_CFG_LET_TERM(var_name) FLTL_CFG_LET_(fltl::lib::cfg::query_terminal_tag, var_name)

#ifndef unbound
#define unbound const static
#endif

namespace fltl { namespace lib {

    // forward declaration
    template <typename AlphaT>
    class CFG;

    namespace cfg {

        /// type used to represent symbols (terminals, non-terminals)
        /// of a grammar.
        typedef int32_t internal_sym_type;

#if 0
        /// tag classes for meta-programming
        class query_builder_tag { };
        class query_non_terminal_tag { };
        class query_terminal_tag { };
#endif

        // forward declarations
        template <typename> class Variable;
        template <typename> class Production;
        template <typename> class OpaqueProduction;
        template <typename> class ProductionBuilder;
        template <typename> class Symbol;
        template <typename> class SymbolString;
        template <typename, typename> class Unbound;
        template <typename AlphaT> class Generator;
        template <typename AlphaT, typename LHS, typename RHS> class Pattern;

        namespace detail {

            // forward declarations
            template <typename AlphaT, const unsigned num_symbols>
            struct SymbolStringAllocator;

            template <typename AlphaT, typename T>
            class PatternElem;

            template <typename AlphaT, typename T0, typename T1>
            class PatternPair;

            template <typename AlphaT>
            class SimpleGenerator;
        }
    }

}}

#include "fltl/lib/cfg/Symbol.hpp"
#include "fltl/lib/cfg/Production.hpp"
#include "fltl/lib/cfg/Variable.hpp"

namespace fltl { namespace lib {

    /// context-free grammar type.
    ///
    /// Assumptions:
    ///     - AlphaT has a strict weak ordering.
    ///     - AlphaT is default constructible
    ///     - AlphaT is copy constructible
    template <typename AlphaT>
    class CFG : protected trait::Uncopyable {
    private:

        // friend declarations
        template <typename, typename> friend class QueryBuilder;
        friend class cfg::Variable<AlphaT>;
        friend class cfg::ProductionBuilder<AlphaT>;
        friend class cfg::Production<AlphaT>;
        friend class cfg::detail::SimpleGenerator<AlphaT>;
        template <typename, const unsigned> friend class cfg::detail::SymbolStringAllocator;

        /// the next variable id that can be assigned, goes toward +inf
        cfg::internal_sym_type next_variable_id;

        /// the next terminal id that can be assigned, goes toward -inf
        cfg::internal_sym_type next_terminal_id;

        /// injective mapping between non-zero negative integers and pointers
        /// to the parameterized alphabet type. the association between
        /// terminals and their representations needs to be maintained.
        helper::Array<AlphaT> terminal_map;
        std::map<AlphaT, cfg::internal_sym_type> terminal_map_inv;

        /// injective mapping between non-zero positive integers and pointers
        /// to the structure containing the productions related to the
        /// variable.
        helper::Array<cfg::Variable<AlphaT> *> variable_map;

        /// number of productions
        unsigned _num_productions;

        /// allocator for variables
        static helper::StorageChain<helper::BlockAllocator<
            cfg::Variable<AlphaT>
        > > variable_allocator;

        /// allocator for productions
        static helper::StorageChain<helper::BlockAllocator<
            cfg::Production<AlphaT>
        > > production_allocator;

        /// temporary generator
        static cfg::Generator<AlphaT> gen_ref;

    public:

#if 0
        /// type tag for specializing
        typedef cfg::query_builder_tag query_builder_tag;
#endif

        /// arbitrary symbol (terminal, non-terminal) of a grammar
        typedef cfg::Symbol<AlphaT> symbol_type;

        /// type of a production builder
        typedef cfg::ProductionBuilder<AlphaT> production_builder_type;

        /// represents a terminal of a grammar
        class terminal_type : public cfg::Symbol<AlphaT> {
        private:
            friend class CFG<AlphaT>;

            explicit terminal_type(const cfg::internal_sym_type _value) throw()
                : cfg::Symbol<AlphaT>(_value)
            { }

        public:

            terminal_type(void) throw()
                : cfg::Symbol<AlphaT>(0)
            { }

            /// return an "unbound" version of this symbol
            /// note: *not* const!!
            cfg::Unbound<AlphaT,terminal_type> operator~(void) throw() {
                return cfg::Unbound<AlphaT,terminal_type>(this);
            }
        };

        /// represents a non-terminal of a grammar
        class variable_type : public cfg::Symbol<AlphaT> {
        private:
            friend class CFG<AlphaT>;

            explicit variable_type(const cfg::internal_sym_type _value) throw()
                : cfg::Symbol<AlphaT>(_value)
            { }

        public:

            variable_type(void) throw()
                : cfg::Symbol<AlphaT>(0)
            { }

            /// return an "unbound" version of this symbol
            /// note: *not* const!!
            cfg::Unbound<AlphaT,variable_type> operator~(void) throw() {
                return cfg::Unbound<AlphaT,variable_type>(this);
            }

            /// making a query where the variable is bound
            template <typename RHS>
            cfg::Pattern<
                AlphaT,
                variable_type,
                cfg::detail::PatternElem<AlphaT, RHS>
            >
            operator->*(RHS rhs) const throw() {
                (void) rhs;

                return cfg::Pattern<
                    AlphaT,
                    variable_type,
                    cfg::detail::PatternElem<AlphaT, RHS>
                >();
            }
        };

        /// represents a production
        typedef cfg::OpaqueProduction<AlphaT> production_type;

        /// string of symbols
        typedef cfg::SymbolString<AlphaT> symbol_string_type;

        /// generator of search results
        typedef cfg::Generator<AlphaT> generator_type;

        /// short forms
        typedef symbol_type sym_t;
        typedef production_builder_type prod_builder_t;
        typedef terminal_type term_t;
        typedef variable_type var_t;
        typedef production_type prod_t;
        typedef symbol_string_type sym_str_t;
        typedef generator_type gen_t;

        /// constructor
        CFG(void) throw()
            : trait::Uncopyable()
            , next_variable_id(1)
            , next_terminal_id(-1)
            , terminal_map()
            , terminal_map_inv()
            , variable_map()
            , _num_productions(0)
        {
            terminal_map.reserve(256U);
            variable_map.reserve(256U);

            terminal_map.append(mpl::Static<AlphaT>::VALUE);
            variable_map.append(variable_allocator->allocate());
        }

        /// destructor
        ~CFG(void) throw() {

            // free the variables
            for(cfg::Variable<AlphaT> *var(variable_map.get(0)), *next_var(0);
                0 != var;
                var = next_var) {

                next_var = var->next;
                variable_map.set(var->id, 0);
                variable_allocator->deallocate(var);
            }
        }

        /// add a new variable to the
        const variable_type add_variable(void) throw() {
            cfg::Variable<AlphaT> *var(variable_allocator->allocate());
            var->init(next_variable_id, variable_map.back());

            cfg::internal_sym_type var_id(next_variable_id);

            ++next_variable_id;
            variable_map.back()->next = var;
            variable_map.append(var);

            variable_type ret(var_id);
            return ret;
        }

        /// get the terminal reference for a particular terminal.
        const terminal_type get_terminal(const AlphaT term) throw() {
            cfg::internal_sym_type &term_id(terminal_map_inv[term]);

            if(0 == term_id) {
                term_id = next_terminal_id;
                --next_terminal_id;
                terminal_map.append(term);
            }

            terminal_type ret(term_id);
            return ret;
        }

        /// add a production to the grammar from a symbol string
        const production_type add_production(
            const variable_type _var,
            symbol_string_type str
        ) throw() {

            cfg::Variable<AlphaT> *var(get_variable(_var));
            cfg::Production<AlphaT> *prod(production_allocator->allocate());

            prod->var = _var;
            prod->symbols.assign(str);

            // make sure the production is unique
            for(cfg::Production<AlphaT> *related_prod(var->first_production);
                0 != related_prod;
                related_prod = related_prod->next) {
                if(related_prod->is_equivalent_to(*prod)) {
                    production_allocator->deallocate(prod);
                    return production_type(related_prod);
                }
            }

            // inductive step: add the production to the current variable
            ++_num_productions;
            var->add_production(prod);
            return production_type(prod);
        }

        /// add a production to the grammar that has the sames symbols as
        /// another production
        inline const production_type add_production(
            const variable_type _var,
            production_type _prod
        ) throw() {
            return add_production(_var, _prod.symbols());
        }

        /// add a production to the grammar from a symbol
        inline const production_type add_production(
            const variable_type _var,
            const symbol_type _sym
        ) throw() {
            return add_production(_var, _sym + epsilon());
        }

        /// add a production to the grammar from a production builder
        inline const production_type add_production(
            const variable_type _var,
            production_builder_type &builder
        ) throw() {
            return add_production(_var, builder.symbols());
        }

        /// remove a production from the grammar
        void remove_production(production_type &_prod) throw() {
            cfg::Production<AlphaT> *prod(_prod.production);
            cfg::Variable<AlphaT> *var(variable_map.get(prod->get(0).value));
            var->remove_production(prod);
            prod->prev = 0;
            prod->next = 0;
            cfg::Production<AlphaT>::release(prod);
            prod = 0;
        }

        /// get the variable representing the empty string, epsilon
        inline const variable_type epsilon(void) const throw() {
            return mpl::Static<variable_type>::VALUE;
        }

        /// get the number of variables in this CFG
        inline unsigned num_variables(void) const throw() {
            return variable_map.size() - 1U;
        }

        /// get the number of productions in the CFG
        inline unsigned num_productions(void) const throw() {
            return _num_productions;
        }

        /// get the number of terminals in the CFG; note: not all terminals
        /// are necessarily reachable
        inline unsigned num_terminals(void) const throw() {
            return terminal_map.size() - 1U;
        }

        /// create a variable generator
        inline generator_type
        search(cfg::Unbound<AlphaT, variable_type> sym) throw() {
            return generator_type(
                this,
                variable_map.get(0),
                0,
                0U,
                reinterpret_cast<void *>(sym.var),
                &(cfg::detail::SimpleGenerator<AlphaT>::find_next_variable)
            );
        }

        /// create a terminal generator
        inline generator_type
        search(cfg::Unbound<AlphaT, terminal_type> sym) throw() {
            return generator_type(
                this,
                0,
                0,
                1U,
                reinterpret_cast<void *>(sym.term),
                &(cfg::detail::SimpleGenerator<AlphaT>::find_next_terminal)
            );
        }

        /// create a production generator
        inline generator_type
        search(cfg::Unbound<AlphaT, production_type> uprod) throw() {
            return generator_type(
                this,
                variable_map.get(0),
                0,
                0U,
                reinterpret_cast<void *>(uprod.prod),
                &(cfg::detail::SimpleGenerator<AlphaT>::find_next_production)
            );
        }

        /// return an empty generator for
        inline generator_type search(cfg::Unbound<AlphaT, symbol_type>) throw() {
            return gen_ref;
        }

        template <typename T>
        inline generator_type &search(const T expr) throw() {
            (void) expr;
            generator_type gen(this);
            gen_ref = gen;
            return gen_ref;
        }

    private:

        inline cfg::Variable<AlphaT> *
        get_variable(const variable_type _var) throw() {
            assert(
                0 < _var.value &&
                _var.value < next_variable_id &&
                "Invalid variable passed to add_production()."
            );

            cfg::Variable<AlphaT> *var(variable_map.get(_var.value));

            assert(
                0 != var &&
                "Invalid variable passed to add_production()."
            );

            return var;
        }

    public:

        void debug(const production_type &prod) throw() {
            if(!prod.valid()) {
                printf("<empty production>\n");
            } else {
                printf("\033[33m%d\033[0m -> ", prod.variable().value);
                symbol_string_type syms(prod.symbols());
                for(unsigned i(0); i < syms.length(); ++i) {
                    if(0 < syms[i].value) {
                        printf("\033[33m%d ", syms[i].value);
                    } else {
                        printf("\033[35m%c ", terminal_map.get(-1 * syms[i].value));
                    }
                }
                printf("\033[0m\n");
            }
        }

        void debug(const symbol_type &sym) throw() {
            if(0 == sym.value) {
                printf("\x27\n");
            } else if(0 < sym.value) {
                printf("\033[33m%d\033[0m\n", sym.value);
            } else {
                printf("\033[35m%c\033[0m\n", terminal_map.get(-1 * sym.value));
            }
        }
    };

    // initialize the static variables
    template <typename AlphaT>
    helper::StorageChain<helper::BlockAllocator<
        cfg::Variable<AlphaT>
    > > CFG<AlphaT>::variable_allocator;

    template <typename AlphaT>
    helper::StorageChain<helper::BlockAllocator<
        cfg::Production<AlphaT>
    > > CFG<AlphaT>::production_allocator(CFG<AlphaT>::variable_allocator);

    template <typename AlphaT>
    cfg::Generator<AlphaT> CFG<AlphaT>::gen_ref;
}}

#include "fltl/lib/cfg/ProductionBuilder.hpp"
#include "fltl/lib/cfg/SymbolString.hpp"
#include "fltl/lib/cfg/OpaqueProduction.hpp"
#include "fltl/lib/cfg/Unbound.hpp"
#include "fltl/lib/cfg/Generator.hpp"

#if 0
namespace fltl { namespace mpl {

    /// Classes for template variables.
    namespace detail {
        template <typename TagT, typename>
        class CFGQueryVar { };
    }

    template <typename T>
    class QueryBuilder<T,fltl::lib::cfg::query_builder_tag> {
    public:
    };

}}
#endif

#endif /* FLTL_LIB_CONTEXTFREEGRAMMAR_HPP_ */
