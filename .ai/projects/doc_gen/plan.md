# Documentation Generation Project Plan

## Purpose & Goals
Document the legacy MUD codebase, its architecture, components, and gameplay, to provide a comprehensive foundation for future modernization and migration efforts. The goal is to enable rapid onboarding, knowledge transfer, and informed planning for the code migration project.

## Major Phases & Deliverables
1. **Discovery & Inventory**
   - Catalog all original source, data, and documentation files.
   - Gather file sizes, types, and directory structure.
   - Generate a dependency graph (e.g., with ctags, cscope, or clangd).
   - Deliverables: `.ai/context/files_old.md` (initial directory without categories or summaries), `.ai/context/files_docs.md`, `.ai/context/dependency_graph.md`, discovery report.
2. **Header File Summarization**
   - Chunk, batch, and summarize all header files (`.h`, `.hh`) to build a high-level overview of system types, interfaces, and dependencies.
   - Use `.ai/context/files_old.md` for file context and update it with new summaries and insights as each file is processed.
   - Reference `.ai/context/dependency_graph.md` for static dependency context.
   - Store batch and module summaries in `.ai/projects/doc_gen/summary_headers_batch_X.md` and `.ai/projects/doc_gen/summary_headers.md`.
3. **Implementation File Summarization**
   - Chunk, batch, and summarize implementation files (`.c`, `.cpp`, `.cc`, etc.), using header summaries for context.
   - Use `.ai/context/files_old.md` for file context and update it with new summaries and insights as each file is processed.
   - Reference `.ai/context/dependency_graph.md` for static dependency context.
   - Store batch and module summaries in `.ai/projects/doc_gen/summary_impl_batch_X.md` and `.ai/projects/doc_gen/summary_impl.md`.
4. **Combine & Integrate Results**
   - Integrate header and implementation summaries to produce module-level and system-level overviews.
   - Update `.ai/docs/component_summaries.md`, `.ai/docs/architecture_overview.md`, and `.ai/docs/system_overview.md` as needed.
5. **Documentation & Planning**
   - Refine and expand documentation describing the current system, architecture, gameplay, and requirements.
   - Develop a plan for the code migration process, including architectural recommendations and migration strategies.
   - Deliverables: Updated documentation corpus in `.ai/docs/`, migration plan, architecture recommendations, and summary documents.

## Workflow & Iteration
- Each phase and batch follows a cycle: review, Q/A, action plan, execution, and documentation update.
- Use `.ai/context/files_old.md` as a living document, updating it with new file summaries and insights as discovered in each stage.
- Use `.ai/context/dependency_graph.md` as a static reference for file/module relationships throughout the documentation process.
- All findings, decisions, and changes are documented in `.ai/docs/` and project logs.
- Use progressive summarization: treat completed summaries as context for future batches.
- Explicitly log open questions, assumptions, and unresolved issues for each batch.
- Define human review checkpoints after major phases (e.g., after headers, after core modules).

## Success Criteria
- All legacy files and documentation are cataloged and summarized.
- The system is fully described and understood, with clear documentation for future migration.
- The migration plan is actionable and informed by thorough analysis.

---

This plan will evolve as the project progresses. Revisit and update as needed.
