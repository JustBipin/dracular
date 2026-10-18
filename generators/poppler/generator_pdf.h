/*
    SPDX-FileCopyrightText: 2004-2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2004 Enrico Ros <eros.kde@email.it>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _OKULAR_GENERATOR_PDF_H_
#define _OKULAR_GENERATOR_PDF_H_

// #include "synctex/synctex_parser.h"

#include <poppler-qt6.h>
#include <poppler-version.h>

#include <QBitArray>
#include <QMutex>
#include <QPointer>
#include <QSet>

#include <memory>

#include <core/annotations.h>
#include <core/document.h>
#include <core/generator.h>
#include <core/printoptionswidget.h>
#include <interfaces/configinterface.h>
#include <interfaces/printinterface.h>
#include <interfaces/saveinterface.h>

#include <unordered_map>

#if HAVE_POPPLER_CORE_OUTPUTDEV
#include "darkreaderoutputdev.h"
#endif

class PDFOptionsPage;
class PopplerAnnotationProxy;

/**
 * @short A generator that builds contents from a PDF document.
 *
 * All Generator features are supported and implemented by this one.
 * Internally this holds a reference to xpdf's core objects and provides
 * contents generation using the PDFDoc object and a couple of OutputDevices
 * called Okular::OutputDev and Okular::TextDev (both defined in gp_outputdev.h).
 *
 * For generating page contents we tell PDFDoc to render a page and grab
 * contents from out OutputDevs when rendering finishes.
 *
 */
class PDFGenerator : public Okular::Generator, public Okular::ConfigInterface, public Okular::PrintInterface, public Okular::SaveInterface
{
    Q_OBJECT
    Q_INTERFACES(Okular::Generator)
    Q_INTERFACES(Okular::ConfigInterface)
    Q_INTERFACES(Okular::PrintInterface)
    Q_INTERFACES(Okular::SaveInterface)

public:
    PDFGenerator(QObject *parent, const QVariantList &args);
    ~PDFGenerator() override;

    // [INHERITED] load a document and fill up the pagesVector
    Okular::Document::OpenResult loadDocumentWithPassword(const QString &filePath, QList<Okular::Page *> &pagesVector, const QString &password) override;
    Okular::Document::OpenResult loadDocumentFromDataWithPassword(const QByteArray &fileData, QList<Okular::Page *> &pagesVector, const QString &password) override;
    void loadPages(QList<Okular::Page *> &pagesVector, int rotation = -1, bool clear = false);
    // [INHERITED] document information
    Okular::DocumentInfo generateDocumentInfo(const QSet<Okular::DocumentInfo::Key> &keys) const override;
    const Okular::DocumentSynopsis *generateDocumentSynopsis() override;
    Okular::FontInfo::List fontsForPage(int page) override;
    const QList<Okular::EmbeddedFile *> *embeddedFiles() const override;
    PageLayout defaultPageLayout() const override;
    bool defaultPageContinuous() const override;
    PageSizeMetric pagesSizeMetric() const override
    {
        return Pixels;
    }
    QAbstractItemModel *layersModel() const override;
    Okular::BackendOpaqueAction::OpaqueActionResult opaqueAction(const Okular::BackendOpaqueAction *action) override;

    // [INHERITED] document information
    bool isAllowed(Okular::Permission permission) const override;

    // [INHERITED] perform actions on document / pages
    QImage image(Okular::PixmapRequest *request) override;

    // [INHERITED] print page using an already configured kprinter
    Okular::Document::PrintError print(QPrinter &printer) override;

    // [INHERITED] reply to some metadata requests
    QVariant metaData(const QString &key, const QVariant &option) const override;

    // [INHERITED] reparse configuration
    bool reparseConfig() override;
    void addPages(KConfigDialog *) override;

    // [INHERITED] text exporting
    Okular::ExportFormat::List exportFormats() const override;
    bool exportTo(const QString &fileName, const Okular::ExportFormat &format) override;

    // [INHERITED] print interface
    Okular::PrintOptionsWidgetInterface *printConfigurationWidget() const override;

    // [INHERITED] save interface
    bool supportsOption(SaveOption) const override;
    bool save(const QString &fileName, SaveOptions options, QString *errorText) override;
    Okular::AnnotationProxy *annotationProxy() const override;

    bool canSign() const override;
    std::pair<Okular::SigningResult, QString> sign(const Okular::NewSignatureData &oData, const QString &rFilename) override;

    Okular::CertificateStore *certificateStore() const override;

    QByteArray requestFontData(const Okular::FontInfo &font) override;

    static void okularToPoppler(const Okular::NewSignatureData &oData, Poppler::PDFConverter::NewSignatureData *pData);

    enum DocumentAdditionalActionType {
        CloseDocument,
        SaveDocumentStart,
        SaveDocumentFinish,
        PrintDocumentStart,
        PrintDocumentFinish,
    };
    Okular::Action *additionalDocumentAction(Okular::Document::DocumentAdditionalActionType type) override;

protected:
    SwapBackingFileResult swapBackingFile(QString const &newFileName, QList<Okular::Page *> &newPagesVector) override;
    bool doCloseDocument() override;
    Okular::TextPage *textPage(Okular::TextRequest *request) override;

private:
    Okular::Document::OpenResult init(QList<Okular::Page *> &pagesVector, const QString &password);

    // fetch annotations from the pdf file and add they to the page
    void addAnnotations(Poppler::Page *popplerPage, Okular::Page *page);
    // fetch the transition information and add it to the page
    void addTransition(Poppler::Page *pdfPage, Okular::Page *page);
    // fetch the poppler page form fields
    QList<Okular::FormField *> getFormFields(Poppler::Page *popplerPage);

    Okular::TextPage *abstractTextPage(const std::vector<std::unique_ptr<Poppler::TextBox>> &text, double height, double width, int rot);

    void resolveMediaLinkReferences(Okular::Page *page);
    void resolveMediaLinkReference(Okular::Action *action);

    bool setDocumentRenderHints();

    // poppler dependent stuff
    std::unique_ptr<Poppler::Document> pdfdoc;

    void xrefReconstructionHandler();

    // misc variables for document info and synopsis caching
    QString documentFilePath;
    bool xrefReconstructed;
    bool hasVisibleOverprint;
    std::optional<Okular::DocumentSynopsis> docSyn;
    mutable bool docEmbeddedFilesDirty;
    mutable QList<Okular::EmbeddedFile *> docEmbeddedFiles;
    int nextFontPage;
    PopplerAnnotationProxy *annotProxy;
    mutable std::unique_ptr<Okular::CertificateStore> certStore;
    // the hash below only contains annotations that were present on the file at open time
    // this is enough for what we use it for
    QHash<Okular::Annotation *, Poppler::Annotation *> annotationsOnOpenHash;

    QBitArray rectsGenerated;

    QPointer<PDFOptionsPage> pdfOptionsPage;

    bool documentHasPassword = false;
    QHash<int, Okular::Action *> m_additionalDocumentActions;
    void setAdditionalDocumentAction(Okular::Document::DocumentAdditionalActionType type, Okular::Action *action);
    std::shared_ptr<void> m_signatureSettingsHandle;

#if HAVE_POPPLER_CORE_OUTPUTDEV
    // BEGIN Dark Reader exact image mask

    // Maximum number of page entries kept in m_darkReaderMaskCache. Bounded
    // LRU: when full, the least recently requested page entry (both mask72
    // and scaledMask) is evicted. Full-entry eviction (rather than dropping
    // only scaledMask and retaining mask72) was chosen because re-priming a
    // base mask is cheap (one 72 DPI secondary render, ~5ms) while keeping
    // only one image per entry keeps peak memory predictable: mask72 is
    // ~0.5MB (72 DPI Grayscale8) and scaledMask is a few MB at typical zooms.
    static constexpr int DarkReaderMaskCacheCapacity = 20;

    // Primes the 72 DPI base mask for @p pageNum via the secondary
    // DarkReaderRenderer, opening it from @p filePath (no-op when @p
    // filePath is empty or @p hasPassword is set; password-protected
    // documents are unsupported, see DarkReaderRenderer::create()).
    // Callers pass snapshots of documentFilePath/documentHasPassword copied
    // under userMutex(). Never holds userMutex itself: the secondary PDFDoc
    // is fully independent of pdfdoc, so the 72 DPI raster (displayPageSlice
    // + bitmap copy) must not serialize against primary raster work.
    // Concurrent secondary renders are still serialized against each other
    // by m_darkReaderMaskMutex (Poppler's core PDFDoc is not documented as
    // thread-safe, so concurrent displayPageSlice calls are avoided even on
    // independent documents).
    void ensureDarkReaderBaseMask(int pageNum, const QString &filePath, bool hasPassword);

    // Scales the cached 72 DPI base mask for @p pageNum to @p targetSize
    // (the actual primary bitmap size, see image()) and records @p fullDpiX
    // / @p fullDpiY as the single-slot cache key for that page. Tile
    // requests reuse the same full-page entry: they pass the full-page DPI
    // and full-page target size, never tile-local values. No-op when no base
    // mask is cached (mask unavailable: callers fall back to recoloring the
    // whole page). Only ever holds m_darkReaderMaskMutex, briefly: the
    // QImage scale itself is ~instant compared to a render.
    void scaleDarkReaderMaskTo(int pageNum, double fullDpiX, double fullDpiY, const QSize &targetSize);

    // LRU bookkeeping for m_darkReaderMaskCache. Call only with
    // m_darkReaderMaskMutex held.
    void touchDarkReaderMaskLru(int pageNum);
    void evictDarkReaderMaskIfNeeded();

    // Secondary, independent PDFDoc used only to drive Dark Reader's
    // mask-generating renders (see darkreaderoutputdev.h for why a second
    // document is needed at all). Created lazily on first use and kept
    // around for as long as it keeps succeeding; reset whenever it fails so
    // the next attempt can retry from scratch (e.g. if the file was briefly
    // unavailable). Guarded by m_darkReaderMaskMutex; never touch it while
    // holding userMutex().
    std::unique_ptr<DarkReaderRenderer> m_darkReaderRenderer;

    // Cached "exclude from recoloring" masks per page.
    // The mask72 is the base mask rendered at a fixed 72 DPI, which is very fast
    // and only done once. scaledMask is dynamically scaled to the actual primary
    // bitmap size (see scaleDarkReaderMaskTo), so consumers need no rescale.
    struct DarkReaderMaskEntry {
        QImage mask72;
        QImage scaledMask;
        double dpiX = 0;
        double dpiY = 0;
    };
    QHash<int, DarkReaderMaskEntry> m_darkReaderMaskCache;
    // Least-recently-used page order for m_darkReaderMaskCache (most recent
    // last); guarded by m_darkReaderMaskMutex.
    QList<int> m_darkReaderMaskLru;
    // Guards m_darkReaderRenderer, m_darkReaderMaskCache and
    // m_darkReaderMaskLru. Deliberately separate from userMutex(): secondary
    // mask renders must not serialize against primary pdfdoc raster work.
    // Never hold both mutexes at once (no nesting anywhere, so no lock
    // ordering to maintain). Mutable so the const metaData() NoRecolorMask
    // read can lock it.
    mutable QMutex m_darkReaderMaskMutex;
    QSet<int> m_pagesWithUnsavedAnnotations;
    // Tracks whether Dark Reader recolor was active at the last reparseConfig().
    // Used to detect mid-session toggles: enabling Dark Reader primes no mask
    // and paint alone can never warm it (only image() primes), so a toggle
    // must invalidate pixmaps to force re-requests (which prime the mask).
    bool m_darkReaderActive = false;
    // END Dark Reader exact image mask
#endif
};

#endif
