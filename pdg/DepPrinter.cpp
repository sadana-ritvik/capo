// dependency graph is in functionwrapper.h!
#include "ProgramDependencies.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"
#include "llvm/Support/TypeName.h"

namespace llvm {
    template <>
    struct DOTGraphTraits<pdg::DepGraphNode *> : public DefaultDOTGraphTraits {
        DOTGraphTraits (bool isSimple = false): DefaultDOTGraphTraits(isSimple) {}

        std::string getNodeLabel(pdg::DepGraphNode *Node, pdg::DepGraphNode *Graph){
            using namespace pdg;
            const InstructionWrapper *instW = Node->getData();

            if(instW == nullptr || instW == NULL){
                errs() <<"instW " << instW << "\n";
                return "null instW";
            }

            std::string Str;
            raw_string_ostream OS(Str);
            switch(instW->getType()) {
                case ENTRY:
                    OS << *instW->getFunction()->getMetadata("dbg");
                    //OS << instW->getFunction()->getSectionPrefix();
                    return ("<<ENTRY>> " + instW->getFunctionName() + " <" + OS.str() + ">");

                case GLOBAL_VALUE:{
                    OS << *instW->getValue();
                    return ("GLOBAL_VALUE:" + OS.str());
                }

                case FORMAL_IN:{
                    llvm::Argument *arg = instW->getArgument();
                    int arg_pos = arg->getArgNo();
                    OS << *instW->getArgument()->getType();
                    return ("FORMAL_IN: " + std::to_string(arg_pos) + " " + OS.str());
                }

                case ACTUAL_IN:{
                    llvm::Argument *arg = instW->getArgument();
                    int arg_pos = arg->getArgNo();
                    OS << *instW->getArgument()->getType();
                    return ("ACTUAL_IN: " + std::to_string(arg_pos) + " " + OS.str() );
                }
                case FORMAL_OUT:{
                    llvm::Argument *arg = instW->getArgument();
                    int arg_pos = arg->getArgNo();
                    OS << *instW->getArgument()->getType();
                    return ("FORMAL_OUT: " + std::to_string(arg_pos)+ " " + OS.str());
                }

                case ACTUAL_OUT:{
                    llvm::Argument *arg = instW->getArgument();
                    int arg_pos = arg->getArgNo();
                    OS << *instW->getArgument()->getType();
                    return ("ACTUAL_OUT: " + std::to_string(arg_pos) + " " + OS.str());
                }

                case PARAMETER_FIELD:{
                    llvm::Argument *arg = instW->getArgument();
                    int arg_pos = arg->getArgNo();
                    OS << *instW->getFieldType() << " arg_pos: " << arg_pos << " - " << "f_id: " << instW->getFieldId();
                    return OS.str();
                }

                    //for pointer node, add a "*" sign before real node content
                    //if multi level pointer, use a loop instead here
                case POINTER_RW:{
                    OS << *instW->getArgument()->getType();
                    return ("POINTER READ/WRITE : *" + OS.str());
                }

                case STRUCT_FIELD: {
                    llvm::Instruction *inst = instW->getInstruction();
                    llvm::AllocaInst *allocaInst = dyn_cast<AllocaInst>(inst);
                    // processing differently when get a struct pointer
                    llvm::StringRef struct_name = "";
                    if (allocaInst->getAllocatedType()->isPointerTy()) {
                        llvm::PointerType *pt = dyn_cast<llvm::PointerType>(allocaInst->getAllocatedType());
                        struct_name = pt->getElementType()->getStructName();
                    } else {
                        struct_name = allocaInst->getAllocatedType()->getStructName();
                    }

                    std::string struct_string = struct_name.str();
                    std::vector<std::string> TYPE_NAMES = {
                            "VoidTy",    ///<  0: type with no size
                            "HalfTy",        ///<  1: 16-bit floating point type
                            "FloatTy",       ///<  2: 32-bit floating point type
                            "DoubleTy",      ///<  3: 64-bit floating point type
                            "X86_FP80Ty",    ///<  4: 80-bit floating point type (X87)
                            "FP128Ty",       ///<  5: 128-bit floating point type (112-bit mantissa)
                            "PPC_FP128Ty",   ///<  6: 128-bit floating point type (two 64-bits, PowerPC)
                            "LabelTy",       ///<  7: Labels
                            "MetadataTy",    ///<  8: Metadata
                            "X86_MMXTy",     ///<  9: MMX vectors (64 bits, X86 specific)
                            "TokenTy",       ///< 10: Tokens

                            // Derived types... see DerivedTypes.h file.
                            // Make sure FirstDerivedTyID stays up to date!
                            "IntegerTy",     ///< 11: Arbitrary bit width integers
                            "FunctionTy",    ///< 12: Functions
                            "StructTy",      ///< 13: Structures
                            "ArrayTy",       ///< 14: Arrays
                            "PointerTy",     ///< 15: Pointers
                            "VectorTy"
                    };
                    llvm::Type *field_type = instW->getFieldType();
                    std::string type_name = TYPE_NAMES.at(field_type->getTypeID());

                    std::string ret_string = "";
                    std::string field_pos = std::to_string(instW->getFieldId());
                    ret_string = struct_string + " (" + type_name + ") : " + std::to_string(instW->getFieldId());

//                    std::vector<std::string> fields_name = struct_fields_map[struct_string.substr(7)];
//                    if (fields_name.empty() == false) {
//                        std::string field_name =  fields_name.at(instW->getFieldId());
//                        ret_string = struct_string + " (" + type_name + ") : " + field_name ;
//                    } else {
//                        std::string field_pos = std::to_string(instW->getFieldId());
//                        ret_string = struct_string + " (" + type_name + ") : " + std::to_string(instW->getFieldId());
//                    }
                    return (ret_string);
                }

                default: {
                    break;
                }
            }

            llvm::Instruction *inst = Node->getData()->getInstruction();

            if (isSimple() && !inst->getName().empty()) {
		//errs() << "ACAC is simple" << "\n";
                return inst->getName().str();
            } else {
		//errs() << "ACAC is not simple" << "\n";
                std::string Str;
                raw_string_ostream OS(Str);
                OS << *inst;
		//errs() << "ACAC :" << OS.str() << "\n";
		//errs() << "ACAC 1:" << *(inst->getDebugLoc())  << "\n";
                return OS.str();
            }
        }
    };

    template <>
    struct DOTGraphTraits<pdg::DepGraph *>
            : public DOTGraphTraits<pdg::DepGraphNode *> {
        DOTGraphTraits (bool isSimple = false)
                : DOTGraphTraits<pdg::DepGraphNode *>(isSimple) {}

        std::string getNodeLabel(pdg::DepGraphNode *Node, pdg::DepGraph *Graph) {
            return DOTGraphTraits<pdg::DepGraphNode *>::getNodeLabel(
                    Node, *(Graph->begin_children()));
        }

    };

    // data dependency graph

    template <>
    struct DOTGraphTraits<pdg::DataDependencyGraph *>
            : public DOTGraphTraits<pdg::DepGraph *> {
        DOTGraphTraits(bool isSimple = false)
                : DOTGraphTraits<pdg::DepGraph *>(isSimple) {}

        static std::string getGraphName(pdg::DataDependencyGraph *) {
            return "Data dependency graph";
        }

        // return IW.getDependencyType() == DATA ?
        //"style=dotted" : "";
        std::string getEdgeAttributes(pdg::DepGraphNode *Node, pdg::DependencyLinkIterator<pdg::InstructionWrapper> &IW, pdg::DataDependencyGraph *PD) {
            using namespace pdg;
            switch (IW.getDependencyType()) {
                case CONTROL:
                    return "label = \"{CONTROL}\"";
                case DATA_GENERAL:
                    return "style=dotted, label = \"{DATA_GENERAL}\"";
                case GLOBAL_VALUE:
                    return "style=dotted, label = \"{GLOBAL}\"";
                case PARAMETER:
                    return "style=dashed, label = \"{PARAMETER}\"";
                case DATA_DEF_USE: {
                    Instruction *pFromInst = Node->getData()->getInstruction();
                    return "style=dotted,label = \"{DEF_USE}\"";
                }
                case DATA_RAW: {
                    // should be getDependentNode
                    Instruction *pInstruction = IW.getDependencyNode()->getInstruction();
                    // pTo Node must be a LoadInst
                    std::string ret_str;
                    if (isa<LoadInst>(pInstruction)) {
                        LoadInst *SI = dyn_cast<LoadInst>(pInstruction);
                        Value *valLI = SI->getPointerOperand();
                        ret_str =
                                "style=dotted,label = \"{RAW} " + valLI->getName().str() + "\"";
                    } else if (isa<CallInst>(pInstruction)) {
                        ret_str = "style=dotted,label = \"{RAW}\"";
                    } else
                        errs() << "incorrect instruction for DATA_RAW node!"
                               << "\n";
                    return ret_str;
                }
                case STRUCT_FIELDS: {
                    return "style=dotted, label=\"{S_FIELD}\", color=\"red\"";
                }
                default:
                    return "style=dotted,label=\"{UNDEFINED}\"";
            }          // end switch
            //return ""; // default ret statement

        }
        std::string getNodeLabel(pdg::DepGraphNode *Node,
                                 pdg::DataDependencyGraph *Graph) {
            return DOTGraphTraits<pdg::DepGraph *>::getNodeLabel(Node, Graph->DDG);
        }
    };

    // control dependency graph
    template <>
    struct DOTGraphTraits<pdg::ControlDependencyGraph *>
            : public DOTGraphTraits<pdg::DepGraph *> {
        DOTGraphTraits(bool isSimple = false)
                : DOTGraphTraits<pdg::DepGraph *>(isSimple) {}

        static std::string getGraphName(pdg::ControlDependencyGraph *) {
            return "Instruction-Level Control dependency graph";
        }

        std::string getNodeLabel(pdg::DepGraphNode *Node,
                                 pdg::ControlDependencyGraph *Graph) {
            return DOTGraphTraits<pdg::DepGraph *>::getNodeLabel(Node, Graph->CDG);
        }

        static std::string
        getEdgeSourceLabel(pdg::DepGraphNode *Node,
                           pdg::DependencyLinkIterator<pdg::InstructionWrapper> EI) {
            //    errs() << "getEdgeSourceLabel(): type = " <<
            //    EI.getDependencyType() << "\n";
            switch (EI.getDependencyType()) {

                default:
                    return "";
            }
        }
    };

    template <>
    struct DOTGraphTraits<pdg::ProgramDependencyGraph *>
            : public DOTGraphTraits<pdg::DepGraph *> {
        DOTGraphTraits(bool isSimple = false)
                : DOTGraphTraits<pdg::DepGraph *>(isSimple) {}

        static std::string getGraphName(pdg::ProgramDependencyGraph *) {
            return "Program Dependency Graph";
        }

        std::string getNodeLabel(pdg::DepGraphNode *Node,
                                 pdg::ProgramDependencyGraph *Graph) {
            return DOTGraphTraits<pdg::DepGraph *>::getNodeLabel(Node, Graph->PDG);
        }


        // take care of the probable display error here
        std::string
        getEdgeAttributes(pdg::DepGraphNode *Node,
                          pdg::DependencyLinkIterator<pdg::InstructionWrapper> &IW,
                          pdg::ProgramDependencyGraph *PD) {
            using namespace pdg;
            switch (IW.getDependencyType()) {
                case CONTROL:
                    return "label = \"{CONTROL}\"";
                case DATA_GENERAL:
                    return "style=dotted, label = \"{data_g}\"";
                    //return "style=dotted, label = \"{DATA_GENERAL}\"";
                case GLOBAL_VALUE:
                    return "style=dotted, label = \"{GLOBAL_VAL}\"";
                case PARAMETER:
                    return "style=dashed, color=\"blue\", label = \"{PARAMETER}\"";
                case DATA_DEF_USE: {
                    Instruction *pFromInst = Node->getData()->getInstruction();
                    return "style=dotted,label = \"{DEF_USE}\" ";
                }
                case DATA_RAW: {
                    Instruction *pInstruction = IW.getDependencyNode()->getInstruction();
                    // pTo Node must be a LoadInst
                    std::string ret_str;
                    if (isa<LoadInst>(pInstruction)) {
                        LoadInst *LI = dyn_cast<LoadInst>(pInstruction);
                        Value *valLI = LI->getPointerOperand();
                        ret_str = "style=dotted,label = \"{RAW} " + valLI->getName().str() + "\"";
                    } else if (isa<CallInst>(pInstruction)) {
                        ret_str = "style=dotted,label = \"{RAW}\"";
                    } else
                        errs() << "incorrect instruction for DATA_RAW node!"
                               << "\n";
                    return ret_str;
                }
                case STRUCT_FIELDS: {
                    return "style=dotted, label=\"{S_FIELD}\", color=\"red\", penwidth=\"2.0\"";
                }
                default:
                    return "style=dotted,label=\"{UNDEFINED}\"";
            }
        }

        std::string getNodeAttributes(pdg::DepGraphNode *Node, pdg::ProgramDependencyGraph *Graph) {
            using namespace pdg;
            const InstructionWrapper *instW = Node->getData();

            if(instW == nullptr || instW == NULL){
                errs() <<"instW " << instW << "\n";
                return "null instW";
            }

            switch(instW->getType()) {
                case ENTRY:
                    return "";

                case GLOBAL_VALUE:{
                    return "";
                }

                case FORMAL_IN:{
                    return "color=\"blue\"";
                }

                case ACTUAL_IN:{
                    return "color=\"blue\"";
                }
                case FORMAL_OUT:{
                    return "color=\"blue\"";
                }

                case ACTUAL_OUT:{
                    return "color=\"blue\"";
                }

                case PARAMETER_FIELD:{
                    return "color=\"blue\"";
                }

                    //for pointer node, add a "*" sign before real node content
                    //if multi level pointer, use a loop instead here
                case POINTER_RW:{
                    return "color=\"red\"";
                }

                case STRUCT_FIELD: {
                    return "";
                }

                default: {
                    return "";
                }
            }
        }

        std::string getGraphProperties(pdg::ProgramDependencyGraph *Graph) {
            return "graph [ splines=true ]";
        }
    };

} // namespace llvm

struct ControlDependencyViewer
        : public DOTGraphTraitsViewer<pdg::ControlDependencyGraph, false> {
    static char ID;
    ControlDependencyViewer()
            : DOTGraphTraitsViewer<pdg::ControlDependencyGraph, false>("cdgraph", ID) {
    }
};

char ControlDependencyViewer::ID = 0;

static RegisterPass<ControlDependencyViewer>
        CDGViewer("view-cdg", "View control dependency graph of function",
                  false, false);

struct ControlDependencyPrinter
        : public DOTGraphTraitsPrinter<pdg::ControlDependencyGraph, false> {
    static char ID;
    ControlDependencyPrinter()
            : DOTGraphTraitsPrinter<pdg::ControlDependencyGraph, false>("cdgragh",
                                                                   ID) {}
};

char ControlDependencyPrinter::ID = 0;
static RegisterPass<ControlDependencyPrinter>
        CDGPrinter("dot-cdg",
                   "Print control dependency graph of function to 'dot' file",
                   false, false);

// DataPrinter
struct DataDependencyViewer
        : public DOTGraphTraitsViewer<pdg::DataDependencyGraph, false> {
    static char ID;
    DataDependencyViewer()
            : DOTGraphTraitsViewer<pdg::DataDependencyGraph, false>("ddgraph", ID) {}
};

char DataDependencyViewer::ID = 0;
static RegisterPass<DataDependencyViewer>
        DdgViewer("view-ddg", "View data dependency graph of function", false,
                  false);

struct DataDependencyPrinter
        : public DOTGraphTraitsPrinter<pdg::DataDependencyGraph, false> {
    static char ID;
    DataDependencyPrinter()
            : DOTGraphTraitsPrinter<pdg::DataDependencyGraph, false>("ddgragh", ID) {}
};

char DataDependencyPrinter::ID = 0;
static RegisterPass<DataDependencyPrinter>
        DdGPrinter("dot-ddg",
                   "Print data dependency graph of function to 'dot' file",
                   false, false);

struct ProgramDependencyPrinter
        : public DOTGraphTraitsPrinter<pdg::ProgramDependencyGraph, false> {
    static char ID;
    ProgramDependencyPrinter()
            : DOTGraphTraitsPrinter<pdg::ProgramDependencyGraph, false>("pdgragh",
                                                                   ID) {}
};

char ProgramDependencyPrinter::ID = 0;
static RegisterPass<ProgramDependencyPrinter>
        PDGPrinter("dot-pdg",
                   "Print instruction-level program dependency graph of "
                           "function to 'dot' file",
                   false, false);
