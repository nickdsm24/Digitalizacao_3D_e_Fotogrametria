$docPath = "C:\Users\Nmpg2\Documents\02_Estudos\02_Faculdade\03_Internet_das_Coisas\Digitalizacao_3D_e_Fotogrametria\docs\relatorio_final_projeto.docx"
$pdfPath = "C:\Users\Nmpg2\Documents\02_Estudos\02_Faculdade\03_Internet_das_Coisas\Digitalizacao_3D_e_Fotogrametria\docs\relatorio_final_projeto.pdf"

$word = New-Object -ComObject Word.Application
$word.Visible = $false
try {
    $doc = $word.Documents.Open($docPath)
    # wdFormatPDF = 17
    $doc.SaveAs([ref]$pdfPath, [ref]17)
    $doc.Close()
    Write-Output "PDF gerado com sucesso: $pdfPath"
} catch {
    Write-Error "Erro ao converter: $_"
} finally {
    $word.Quit()
}
